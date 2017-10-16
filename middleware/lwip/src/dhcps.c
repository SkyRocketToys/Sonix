#include <stdint.h>
#include <string.h>
#include "lwip/udp.h"
#include "lwip/dhcps.h"
#include "lwip/inet.h"
#include <nonstdlib.h>

static struct udp_pcb  *pcb_dhcps;
static struct dhcps_config svrconf;
static struct client_info g_clients[DHCPS_IPADDR_NUM];

static void dhcps_dump_msg(struct dhcp_msg *msg){
	DHCPS_PRINT_QUEUE(" ===== Dhcp message dump =====\n");
	DHCPS_PRINT_QUEUE("Opcode = 0x%x\n", msg->op);
	DHCPS_PRINT_QUEUE("Hw type = 0x%x\n", msg->htype);
	DHCPS_PRINT_QUEUE("Hw length = 0x%x\n", msg->hlen);
	DHCPS_PRINT_QUEUE("Hops = 0x%x\n",msg->hops);
	DHCPS_PRINT_QUEUE("Transcation ID = 0x%x\n",ntohl(msg->xid));
	DHCPS_PRINT_QUEUE("Seconds = 0x%x\n",ntohs(msg->secs));
	DHCPS_PRINT_QUEUE("Flags = 0x%x\n",ntohs(msg->flags));
	DHCPS_PRINT_QUEUE("Client ip addr = 0x%x\n",ntohl(msg->ciaddr));
	DHCPS_PRINT_QUEUE("Your ip addr = 0x %x\n",ntohl(msg->yiaddr));
	DHCPS_PRINT_QUEUE("Next server ip addr = 0x%x",ntohl(msg->siaddr));
	DHCPS_PRINT_QUEUE("Relay agent ip addr = 0x%x",ntohl(msg->giaddr));
    DHCPS_PRINT_QUEUE("Client MAC : %02X-%02X-%02X-%02X-%02X-%02X\n", msg->chaddr[0], msg->chaddr[1], msg->chaddr[2], msg->chaddr[3], msg->chaddr[4], msg->chaddr[5]);
}

uint8_t* dhcps_option_get(struct dhcp_msg *msg, int code){
	uint8_t * optptr = NULL;
	int rem_len = 0;
	int opt_tlen = 0;

	optptr = msg->options;
	rem_len = sizeof(msg->options);
	while(rem_len > 0){
		if(optptr[OPT_OFFSET_CODE]==DHCP_OPTION_PADDING){
			optptr++;
			rem_len--;
			continue;
		}

		if (optptr[OPT_OFFSET_CODE] == code) {
			return optptr + OPT_OFFSET_DATA;
		}
		opt_tlen = 2 + optptr[OPT_OFFSET_LEN];
		rem_len -= opt_tlen;

		if(rem_len < 0){
			return NULL;
		}
		optptr += opt_tlen;
	}
	return NULL;
}

static uint8_t * add_msg_type(uint8_t *optptr, uint8_t type)
{
    *optptr++ = DHCP_OPTION_MSG_TYPE;
    *optptr++ = 1;
    *optptr++ = type;
    return optptr;
}
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
static uint8_t * add_server_options(uint8_t *optptr)
{

	uint32_t broadcast = (~svrconf.subnet)|svrconf.ip;
	uint16_t mtu = htons(1500);
	uint32_t lease_time = htonl(svrconf.lease_time);
	struct in_addr addr;

    *optptr++ = DHCP_OPTION_SUBNET_MASK;
    *optptr++ = 4;      //len
	memcpy(optptr, &svrconf.subnet, 4);
	optptr+=4;

    *optptr++ = DHCP_OPTION_LEASE_TIME;
    *optptr++ = 4;      //len
	memcpy(optptr, &lease_time, 4);
	optptr+=4;

    *optptr++ = DHCP_OPTION_SERVER_ID;
    *optptr++ = 4;      //len
	memcpy(optptr, &svrconf.ip, 4);
	optptr+=4;

    *optptr++ = DHCP_OPTION_BROADCAST_ADDRESS;
    *optptr++ = 4;      //len
	memcpy(optptr, &broadcast, 4);
	optptr+=4;

    *optptr++ = DHCP_OPTION_INTERFACE_MTU;
    *optptr++ = 2;      //len
	memcpy(optptr, &mtu, 2);
	optptr+=2;

    //*optptr++ = DHCP_OPTION_PERFORM_ROUTER_DISCOVERY;
    //*optptr++ = 1;      //len
    //*optptr++ = 0x00;   //dont do router discovery
#if CONFIG_DHCP_OPTION_DNS_SERVER
    *optptr++ = DHCP_OPTION_DNS_SERVER;
    *optptr++ = 4;      //len
	inet_aton("129.219.13.81", &addr);
	memcpy(optptr, &addr.s_addr, 4);
	optptr+=4;
#endif

#if CONFIG_DHCP_OPTION_ROUTER
    *optptr++ = DHCP_OPTION_ROUTER;
    *optptr++ = 4;      //len
	memcpy(optptr, &svrconf.ip, 4);
	optptr+=4;
#endif

#if CONFIG_DHCP_OPTION_DOMAIN_NAME
    *optptr++ = DHCP_OPTION_DOMAIN_NAME;
    *optptr++ = 5;      //len
	memcpy(optptr, "local",5);
	optptr+=5;
#endif

    //disable microsoft netbios over tcp
    *optptr++ = 43;     //vendor specific
    *optptr++ = 6;      //length of embedded option

    *optptr++ = 0x01;   //vendor specific (microsoft disable netbios over tcp)
    *optptr++ = 4;      //len
    *optptr++ = 0x00;
    *optptr++ = 0x00;
    *optptr++ = 0x00;
    *optptr++ = 0x02;           //disable=0x02,  enable = 0x00
    return optptr;
}
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
static uint8_t * add_end(uint8_t *optptr)
{
    *optptr++ = DHCP_OPTION_END;
    return optptr;
}
static void create_msg(struct dhcp_msg *new_msg, struct dhcp_msg *msg){
	memset( new_msg, 0, sizeof(struct dhcp_msg));
	new_msg->op = DHCP_REPLY;
	new_msg->htype = DHCP_HTYPE_ETHERNET;
	new_msg->hlen = DHCP_HLEN_ETHERNET;
	new_msg->xid = msg->xid;
	memcpy(new_msg->chaddr, msg->chaddr, sizeof(new_msg->chaddr));
	new_msg->flags = msg->flags;
    new_msg->giaddr = msg->giaddr;
    new_msg->ciaddr = msg->ciaddr;
	new_msg->magic = htonl(DHCP_MAGIC);

}

static time_t get_current_time(void){
	struct timeval cur_tv;

	gettimeofday(&cur_tv, NULL);

	return cur_tv.tv_sec;
}

static struct client_info* find_client_by_mac(uint8_t *mac)
{
	unsigned i;

	for (i = 0; i < svrconf.ip_num; i++)
		if (memcmp(g_clients[i].mac, mac, 6)==0)
			return &g_clients[i];

	return NULL;
}

static struct client_info* find_client_by_ip(uint32_t ip)
{
	unsigned i;

	for (i = 0; i < svrconf.ip_num; i++)
		if (g_clients[i].ip == ip)
			return &g_clients[i];

	return NULL;
}

uint8_t * get_mac_by_ip(uint32_t ip)
{
	struct client_info *client = NULL; 


	client = find_client_by_ip(ip);
	
	/* No entry in dhcp client table */
	if(!client){

		DHCPS_PRINT_QUEUE("Find ip 0x%x client 0x%x in %s\n", ip, client, __FUNCTION__);

		dhcps_client_display();

		return NULL;
	}

	/* dhcp client expire */
	//if(client->expires < get_current_time())
	//	return NULL;

	return client->mac;

}

/* True if a lease has expired */
int is_expired_client(struct client_info *client)
{
    return (client->expires < get_current_time());
}

static uint32_t search_free_ip(uint8_t *mac){
	uint32_t addr, free_ip;
	struct client_info *oldest_client = NULL, *client = NULL;

	addr = svrconf.start_ip;

	while(addr <= svrconf.end_ip){
		free_ip = htonl(addr);

		client = find_client_by_ip(free_ip);

		if(!client){
			return free_ip;
		}else{
			if(!oldest_client || client->expires < oldest_client->expires)
				oldest_client = client;
		}
		addr++;
	}

	 if (oldest_client && is_expired_client(oldest_client)) {
        return oldest_client->ip;
    }

	return 0;

}

static struct client_info *oldest_expired_client(void)
{
    struct client_info *oldest_client = NULL;
    uint32_t oldest_time = get_current_time();
    unsigned i;

    for (i = 0; i < svrconf.ip_num; i++) {
        if (g_clients[i].expires < oldest_time) {
            oldest_time =g_clients[i].expires;
            oldest_client = &g_clients[i];
        }
    }
    return oldest_client;
}


struct client_info* add_client(const uint8_t *chaddr, uint32_t yiaddr, uint32_t leasetime){
    struct client_info *oldest = NULL, *client = NULL;

	/* Find old entry and clear it */
    client = find_client_by_ip(yiaddr);
	if(client){
		 memset(client, 0, sizeof(struct client_info));
	}

	/* Search the oldest entry to record it */
    oldest = oldest_expired_client();
    if (oldest) {
        memset(oldest, 0, sizeof(*oldest));
        memcpy(oldest->mac, chaddr, 6);
        oldest->ip = yiaddr;
        oldest->expires = get_current_time() + leasetime;
    }

    return oldest;
}


static void send_offer( struct dhcp_msg *msg){
	struct pbuf *p;
	struct client_info *client = NULL;
	struct dhcp_msg *new_msg = NULL;
	uint8_t *optptr_end = NULL;

	p = pbuf_alloc(PBUF_TRANSPORT,sizeof(struct dhcp_msg),PBUF_RAM);
	new_msg = p->payload;
	create_msg(new_msg, msg);

	client = find_client_by_mac(msg->chaddr);

	if(client){
		new_msg->yiaddr = client->ip;
		DHCPS_PRINT_QUEUE("Find exist entry with %s (%02X-%02X-%02X-%02X-%02X-%02X)\n",
		inet_ntoa(new_msg->yiaddr), new_msg->chaddr[0], new_msg->chaddr[1], new_msg->chaddr[2], new_msg->chaddr[3], new_msg->chaddr[4], new_msg->chaddr[5]);
	}else{
		new_msg->yiaddr = search_free_ip(msg->chaddr);
		if(new_msg->yiaddr){
			DHCPS_PRINT_QUEUE("Find free entry with IP %s (%02X-%02X-%02X-%02X-%02X-%02X)\n",
					inet_ntoa(new_msg->yiaddr), new_msg->chaddr[0], new_msg->chaddr[1], new_msg->chaddr[2], new_msg->chaddr[3], new_msg->chaddr[4], new_msg->chaddr[5]);
		}
	}
	/* No ip can use and do nothing */
	if(!new_msg->yiaddr){
		DHCPS_PRINT_QUEUE("NO Free IP can use\n");
		goto offer_end;
	}


	client = add_client(new_msg->chaddr, new_msg->yiaddr, svrconf.offer_time);

	if(!client){
		DHCPS_PRINT_QUEUE("Add new client netry in table failed (%02X-%02X-%02X-%02X-%02X-%02X)\n",
		new_msg->chaddr[0], new_msg->chaddr[1], new_msg->chaddr[2], new_msg->chaddr[3], new_msg->chaddr[4], new_msg->chaddr[5]);
		goto offer_end;
	}

	DHCPS_PRINT_QUEUE("Sending OFFER with IP %s (%02X-%02X-%02X-%02X-%02X-%02X)\n", 
		inet_ntoa(new_msg->yiaddr), new_msg->chaddr[0], new_msg->chaddr[1], new_msg->chaddr[2], new_msg->chaddr[3], new_msg->chaddr[4], new_msg->chaddr[5]);

	/* Add dhcp options */
	optptr_end = add_msg_type(new_msg->options, DHCPOFFER);
	optptr_end = add_server_options(optptr_end);
	add_end(optptr_end);
	udp_sendto(pcb_dhcps, p, IP_ADDR_BROADCAST, DHCPS_CLIENT_PORT);

offer_end :
	pbuf_free(p);
	return;
}

static void send_ack( struct dhcp_msg *msg, uint32_t requested_ip){
	struct pbuf *p;
	struct dhcp_msg *new_msg = NULL;
	uint8_t *optptr_end = NULL;
	uint8_t *msg_type = NULL;

	p = pbuf_alloc(PBUF_TRANSPORT,sizeof(struct dhcp_msg),PBUF_RAM);
	msg_type = dhcps_option_get(msg, DHCP_OPTION_MSG_TYPE);
	new_msg = p->payload;
	create_msg(new_msg, msg);

	new_msg->yiaddr = requested_ip;

	optptr_end = add_msg_type(new_msg->options, DHCPACK);
	optptr_end = add_server_options(optptr_end);
	add_end(optptr_end);

	udp_sendto(pcb_dhcps, p, IP_ADDR_BROADCAST, DHCPS_CLIENT_PORT);
	switch(*msg_type){
		case DHCPREQUEST:
			add_client(new_msg->chaddr, new_msg->yiaddr, svrconf.lease_time);
			DHCPS_PRINT_QUEUE("Sending ACK with IP %s (%02X-%02X-%02X-%02X-%02X-%02X)\n", inet_ntoa(new_msg->yiaddr),
					new_msg->chaddr[0], new_msg->chaddr[1], new_msg->chaddr[2], new_msg->chaddr[3], new_msg->chaddr[4], new_msg->chaddr[5]);
			break;
		case DHCPINFORM:
			DHCPS_PRINT_QUEUE("Sending ACK with IP %s (%02X-%02X-%02X-%02X-%02X-%02X)\n", inet_ntoa(new_msg->ciaddr),
					new_msg->chaddr[0], new_msg->chaddr[1], new_msg->chaddr[2], new_msg->chaddr[3], new_msg->chaddr[4], new_msg->chaddr[5]);
			break;
		default:
			break;
	}
	pbuf_free(p);
}


static void send_nak( struct dhcp_msg *msg, uint32_t requested_ip){
	struct pbuf *p;
	struct dhcp_msg *new_msg = NULL;
	uint8_t *optptr_end = NULL;

	p = pbuf_alloc(PBUF_TRANSPORT,sizeof(struct dhcp_msg),PBUF_RAM);
	new_msg = p->payload;
	create_msg(new_msg, msg);

	new_msg->yiaddr = requested_ip;
	DHCPS_PRINT_QUEUE("Sending NAK with IP %s (%02X-%02X-%02X-%02X-%02X-%02X)\n", 
		inet_ntoa(new_msg->yiaddr), new_msg->chaddr[0], new_msg->chaddr[1], new_msg->chaddr[2], new_msg->chaddr[3], new_msg->chaddr[4], new_msg->chaddr[5]);

	optptr_end = add_msg_type(new_msg->options, DHCPNAK);
	add_end(optptr_end);

	udp_sendto(pcb_dhcps, p, IP_ADDR_BROADCAST, DHCPS_CLIENT_PORT);
	pbuf_free(p);
}

static void handle_dhcp(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, uint16_t port)
{
	struct dhcp_msg *msg=NULL;
	uint8_t *msg_type = NULL;
	struct client_info * client = NULL;
	uint32_t requested_ip = 0;
	uint8_t	*requested_ip_opt = NULL;


    if (p == NULL)
        return;

    if (p->next != NULL)
    {
		DHCPS_PRINT_QUEUE("Multiple pbuf get in dhcp server\n");
		pbuf_free(p);
		return;
    }

	msg = p->payload;

	msg_type = dhcps_option_get(msg, DHCP_OPTION_MSG_TYPE);

	DHCPS_PRINT_QUEUE("Get dhcp message type = 0x%x (%02X-%02X-%02X-%02X-%02X-%02X)\n", *msg_type, msg->chaddr[0], msg->chaddr[1], msg->chaddr[2], msg->chaddr[3], msg->chaddr[4], msg->chaddr[5]);

	switch(*msg_type){
		case DHCPDISCOVER:
			send_offer(msg);
			break;
		case DHCPREQUEST:
			client = find_client_by_mac(msg->chaddr);
			requested_ip_opt = dhcps_option_get(msg, DHCP_OPTION_REQ_IPADDR);
			if(requested_ip_opt){
				/* Get ip from request option */
				memcpy(&requested_ip, requested_ip_opt, 4);
				DHCPS_PRINT_QUEUE("Get requested ip %s from option(%02X-%02X-%02X-%02X-%02X-%02X)\n", inet_ntoa(requested_ip),
					msg->chaddr[0], msg->chaddr[1], msg->chaddr[2], msg->chaddr[3], msg->chaddr[4], msg->chaddr[5]);
			}else{
				/* Get ip from client address */
				requested_ip = msg->ciaddr;
				DHCPS_PRINT_QUEUE("Get requested ip %s from ciaddr(%02X-%02X-%02X-%02X-%02X-%02X)\n", inet_ntoa(requested_ip),
					msg->chaddr[0], msg->chaddr[1], msg->chaddr[2], msg->chaddr[3], msg->chaddr[4], msg->chaddr[5]);
			}

			if(!requested_ip){
				DHCPS_PRINT_QUEUE("Ignore the DHCP REQUEST, (%02X-%02X-%02X-%02X-%02X-%02X)\n",
					msg->chaddr[0], msg->chaddr[1], msg->chaddr[2], msg->chaddr[3], msg->chaddr[4], msg->chaddr[5]);
				break;
			}

			if(client && requested_ip == client->ip){
				send_ack(msg, requested_ip);
				break;
			}
			send_nak(msg, requested_ip);
			break;
		case DHCPDECLINE:
			break;
		case DHCPRELEASE:
			client = find_client_by_mac(msg->chaddr);
			if(client&&(msg->ciaddr == client->ip)){
				client->expires = get_current_time();
				DHCPS_PRINT_QUEUE("Release IP %s (%02X-%02X-%02X-%02X-%02X-%02X)\n", inet_ntoa(client->ip), 
					msg->chaddr[0], msg->chaddr[1], msg->chaddr[2], msg->chaddr[3], msg->chaddr[4], msg->chaddr[5]);
			}

			break;
		case DHCPINFORM:
			send_ack(msg, msg->yiaddr);
			break;
		default:
			break;
	}
    pbuf_free(p);
}

void dhcps_config_init(void){
	memset(&svrconf, 0, sizeof(svrconf));
	svrconf.ip = inet_addr(DHCPS_SERVER_IP);
	svrconf.port = DHCPS_SERVER_PORT;
	svrconf.ip_num = DHCPS_IPADDR_NUM;
	svrconf.subnet = inet_addr(DHCPS_SUBNET_MAST);
	svrconf.start_ip = ntohl(inet_addr(DHCPS_START_IP));
	svrconf.end_ip = svrconf.start_ip + svrconf.ip_num - 1;
	svrconf.lease_time = DHCPS_LEASE_TIME;
	svrconf.offer_time = DHCPS_OFFER_TIME;
}

void dhcps_init(void)
{
	err_t err=0;

	dhcps_config_init();

	/* Init dhcp server clint info table */
	memset(g_clients, 0, sizeof(g_clients));

    pcb_dhcps = udp_new();

	if(!pcb_dhcps){
		DHCPS_PRINT_QUEUE(" dhcps pcb new failed.\n");
		DHCPS_PRINT_QUEUE(" Dhcp server init failed\n");
		return;
	}

    err = udp_bind(pcb_dhcps, IP_ADDR_ANY, DHCPS_SERVER_PORT);

	if(err != ERR_OK){
		DHCPS_PRINT_QUEUE(" dhcps pcb bind failed. err = %d\n", err);
		DHCPS_PRINT_QUEUE(" Dhcp server init failed\n");
		udp_remove(pcb_dhcps);
		return;
	}

    udp_recv(pcb_dhcps, handle_dhcp, NULL);
}

void dhcps_deinit(void)
{
    udp_remove(pcb_dhcps);
}

void dhcps_client_display(void){
	int i = 0, cur_time = 0, expires = 0;
	struct client_info *client = NULL;

	cur_time = get_current_time();

	print_msg_queue("\nClient Table Display(Time : %u) :\n", cur_time);
	for(i=0;i<svrconf.ip_num;i++){
		client = g_clients + i;

		if(!(client->expires))
			continue;

		expires = client->expires-cur_time;

		print_msg_queue("%d	",i+1);
		print_msg_queue("%s	",inet_ntoa(client->ip));
		print_msg_queue("%02X-%02X-%02X-%02X-%02X-%02X ", client->mac[0], client->mac[1], client->mac[2], client->mac[3], client->mac[4], client->mac[5]);
		print_msg_queue("%6d%s\n", expires, (expires>0)?"":"(expired)");
	}
}

// ----- HotGen -----
void dhcps_get_client_mac(unsigned char *buf) {
	int i = 0, cur_time = 0, expires = 0;
	struct client_info *client = NULL;

	cur_time = get_current_time();

	for(i = 0; i < svrconf.ip_num; i++) {
		client = g_clients + i;

		if(!(client->expires))
			continue;

		expires = client->expires - cur_time;

		// Do we need this?
		//if (expires <= 0)
		//	continue;

		buf[0] = client->mac[0];
		buf[1] = client->mac[1];
		buf[2] = client->mac[2];
		buf[3] = client->mac[3];
		buf[4] = client->mac[4];
		buf[5] = client->mac[5];
		return;
	}

	buf[0] = 0;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;
	buf[4] = 0;
	buf[5] = 0;
}
// ----- HotGen -----

void dhcps_display(void){
	struct ip_addr svr_ip, subnet, start_ip, end_ip;

	svr_ip.addr = svrconf.ip;
	subnet.addr = svrconf.subnet;
	start_ip.addr = ntohl(svrconf.start_ip);
	end_ip.addr = ntohl(svrconf.end_ip);

	print_msg_queue("\n=== DHCP Server Info Display ===\n\n");
	print_msg_queue("Server IP : %s\n", inet_ntoa(svr_ip));
	print_msg_queue("Server PORT : %u\n", svrconf.port);
	print_msg_queue("Server SUBNET : %s\n", inet_ntoa(subnet));
	print_msg_queue("Max Client Number : %u\n", svrconf.ip_num);
	print_msg_queue("Client Start address : %s\n", inet_ntoa(start_ip));
	print_msg_queue("Client End address : %s\n", inet_ntoa(end_ip));
	print_msg_queue("Client Lease Time : %u\n", svrconf.lease_time);
	print_msg_queue("Client Offer Time : %u\n", svrconf.offer_time);

	dhcps_client_display();
	return;
}
