#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/def.h"
#include "netif/etharp.h"

/*-----------------------------------------------------------------------------------*/
/*
 * tapif_input():
 *
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface.
 *
 */
/*-----------------------------------------------------------------------------------*/
void
wifiif_input(struct netif *netif, unsigned char *pData, unsigned int len)
{
  struct tapif *tapif;
  struct eth_hdr *ethhdr;
  struct pbuf *p;


  p = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
  if (p == NULL) return;//DummyS


  memcpy(p->payload, pData, len);
  p->len = len;
  p->tot_len = len;
  ethhdr = (struct eth_hdr *)p->payload;

  switch(htons(ethhdr->type)) {
	  /* IP or ARP packet? */
	  case ETHTYPE_IP:
	  case ETHTYPE_ARP:
#if PPPOE_SUPPORT
		  /* PPPoE packet? */
	  case ETHTYPE_PPPOEDISC:
	  case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
		  /* full packet send to tcpip_thread to process */
		  if (netif->input(p, netif) != ERR_OK) {
			  LWIP_DEBUGF(NETIF_DEBUG, ("wifiif_input: IP input error\n"));
			  pbuf_free(p);
			  p = NULL;
		  }
    break;
  default:
    pbuf_free(p);
    break;
  }
}
