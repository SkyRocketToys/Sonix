#ifndef __DHCPS_H__
#define __DHCPS_H__

#define DHCPS_PRINT(fmt, args...) print_msg("[DHCPS]%s : "fmt, __func__,  ##args)
#define DHCPS_PRINT_QUEUE(fmt, args...) print_msg_queue("[DHCPS]%s : "fmt, __func__,##args)

/* DHCP SERVER Configuration */
#define DHCPS_SERVER_IP		"192.168.99.1"
#define DHCPS_SUBNET_MAST   "255.255.255.0"
#define DHCPS_START_IP		"192.168.99.2"
#define DHCPS_IPADDR_NUM	16
#define DHCPS_SERVER_PORT	67
#define DHCPS_CLIENT_PORT	68
#define DHCPS_LEASE_TIME	(2*60*60)
#define DHCPS_OFFER_TIME	(60)

#define DHCP_MAGIC              0x63825363

#define DHCPS_STATE_OFFER 		1
#define DHCPS_STATE_DECLINE		2
#define DHCPS_STATE_ACK 		3
#define DHCPS_STATE_NAK			4
#define DHCPS_STATE_IDLE 		5

#define BOOTP_BROADCAST         0x8000

#define DHCP_REQUEST            1
#define DHCP_REPLY              2
#define DHCP_HTYPE_ETHERNET     1
#define DHCP_HLEN_ETHERNET      6
#define DHCP_MSG_LEN            236


#define DHCPDISCOVER	1
#define DHCPOFFER		2
#define DHCPREQUEST		3
#define DHCPDECLINE		4
#define DHCPACK			5
#define DHCPNAK			6
#define DHCPRELEASE		7
#define DHCPINFORM		8

#define DHCP_OPTION_PADDING						0
#define DHCP_OPTION_SUBNET_MASK                 1
#define DHCP_OPTION_ROUTER                      3
#define DHCP_OPTION_DNS_SERVER                  6
#define DHCP_OPTION_DOMAIN_NAME                 15
#define DHCP_OPTION_INTERFACE_MTU               26
#define DHCP_OPTION_BROADCAST_ADDRESS           28
#define DHCP_OPTION_PERFORM_ROUTER_DISCOVERY    31
#define DHCP_OPTION_REQ_IPADDR                  50
#define DHCP_OPTION_LEASE_TIME                  51
#define DHCP_OPTION_MSG_TYPE                    53
#define DHCP_OPTION_SERVER_ID                   54
#define DHCP_OPTION_REQ_LIST                    55
#define DHCP_OPTION_END                         255

#define OPT_OFFSET_CODE     0
#define OPT_OFFSET_LEN      1
#define OPT_OFFSET_DATA     2

/* Enable dhcp option */
#define CONFIG_DHCP_OPTION_DNS_SERVER	1
#define CONFIG_DHCP_OPTION_ROUTER	0
#define CONFIG_DHCP_OPTION_DOMAIN_NAME	1

struct dhcps_config{
	uint32_t ip;
	uint16_t port;
	uint32_t start_ip;
	uint32_t end_ip;
	uint32_t subnet;
	uint32_t ip_num;
	uint32_t lease_time;
	uint32_t offer_time;
};

struct client_info{
	uint32_t ip;
	uint8_t mac[6];
	uint32_t expires;
};

struct dhcp_msg {
	uint8_t  op;      /* 0: Message opcode/type */
	uint8_t  htype;    /* 1: Hardware addr type (net/if_types.h) */
	uint8_t  hlen;     /* 2: Hardware addr length */
	uint8_t  hops;     /* 3: Number of relay agent hops from client */
	uint32_t xid;      /* 4: Transaction ID */
	uint16_t secs;     /* 8: Seconds since client started looking */
	uint16_t flags;    /* 10: Flag bits */
	uint32_t ciaddr;  /* 12: Client IP address (if already in use) */
	uint32_t yiaddr;  /* 16: Client IP address */
	uint32_t siaddr;  /* 18: IP address of next server to talk to */
	uint32_t giaddr;  /* 20: DHCP relay agent IP address */
	unsigned char chaddr [16];  /* 24: Client hardware address */
	char sname [64];    /* 40: Server name */
	char file [128];  /* 104: Boot filename */
	uint32_t magic;
	unsigned char options [308];
}__attribute__ ((packed));

void dhcps_init(void);

void dhcps_deinit(void);

void dhcps_display(void);

uint8_t * get_mac_by_ip(uint32_t ip);


#endif
