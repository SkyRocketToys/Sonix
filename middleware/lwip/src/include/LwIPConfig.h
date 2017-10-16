#ifndef LWIP_CONFIG_H
#define LWIP_CONFIG_H

/* IP layer initialize*/
#define LWIP_PORT_INIT_IPADDR(addr)   IP4_ADDR((addr), 192,168,99,1)
#define LWIP_PORT_INIT_GW(addr)       IP4_ADDR((addr), 192,168,99,1)
#define LWIP_PORT_INIT_NETMASK(addr)  IP4_ADDR((addr), 255,255,255,0)




#endif /* LWIP_CONFIG_H */
