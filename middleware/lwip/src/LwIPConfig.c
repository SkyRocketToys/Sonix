#include <FreeRTOS.h>
#include "lwip/sockets.h"
#include "lwip/err.h"
#include "lwip/netif.h"
#include "LwIPConfig.h"
#include "lwip/dhcp.h"
#include "lwip/ip_addr.h"
#include <sys/time.h>
#include <generated/snx_sdk_conf.h>


/* Extern */
err_t ethernetif_init(struct netif *xNetIf);
err_t tcpip_input(struct pbuf *p, struct netif *inp);

/* Global */
struct netif EMAC_if;
char mydata[1024];

/* Called from the TCP/IP thread. */
void LwIPConfig(void *pvParameters) {
	ip_addr_t xIPAddr, xNetMask, xGateway;

	/* Set up the network interface. */
	ip_addr_set_zero(&xGateway);
	ip_addr_set_zero(&xIPAddr);
	ip_addr_set_zero(&xNetMask);


#if CONFIG_MODULE_WIFI_SUPPORT
#if CONFIG_WIFI_MODE_AP
	/* Setup Static IP for wifi AP mode */
	LWIP_PORT_INIT_GW(&xGateway);
	LWIP_PORT_INIT_IPADDR(&xIPAddr);
	LWIP_PORT_INIT_NETMASK(&xNetMask);
	print_msg("Starting lwIP, local interface IP is %s\n", ip_ntoa(&xIPAddr));
#endif //CONFIG_WIFI_MODE_AP
#endif//CONFIG_MODULE_WIFI_SUPPORT

	netif_set_default(
			netif_add(&EMAC_if, &xIPAddr, &xNetMask, &xGateway, NULL,
					ethernetif_init, tcpip_input));
	netif_set_up(&EMAC_if);

#if CONFIG_MODULE_WIFI_SUPPORT 
#if CONFIG_WIFI_MODE_AP
	dhcps_init();
#endif //CONFIG_WIFI_MODE_AP
#else
	dhcp_start(&EMAC_if);
#endif //CONFIG_MODULE_WIFI_SUPPORT
}

void dhcp_wait(){
	while (EMAC_if.ip_addr.addr == 0) {
		vTaskDelay(50/portTICK_PERIOD_MS);
	}
	print_msg("Starting lwIP, local interface IP is %s\n", ip_ntoa(&EMAC_if.ip_addr.addr));
}

int DhcpWaitAddTimeout(){
	struct timeval cur_tv;
	struct timeval las_tv;
	
	gettimeofday(&las_tv,NULL);	
	while (EMAC_if.ip_addr.addr == 0) {
 		gettimeofday(&cur_tv,NULL);
        if((cur_tv.tv_sec-las_tv.tv_sec) >=30) 
		{
            print_msg("Get DHCP TimeOut\n"); 
			return -1;   
		}
		vTaskDelay(50/portTICK_PERIOD_MS);
	}
	return 0;
	print_msg("Starting lwIP, local interface IP is %s\n", ip_ntoa(&EMAC_if.ip_addr.addr));
}

int get_local_ip(char *ip, int len)
{
	strncpy(ip, ip_ntoa(&EMAC_if.ip_addr.addr), len);
	return 0;
}

int get_router_ip(char *router, int len)
{
	strncpy(router, ip_ntoa(&EMAC_if.gw.addr), len);
	return 0;
}

