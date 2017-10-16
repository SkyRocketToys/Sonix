#ifndef _CMD_NET_H_
#define _CMD_NET_H_

int cmd_net_netinfo(int argc, char* argv[]);
int cmd_net_dhcpsinfo(int argc, char* argv[]);
/* Struct with settings for each throughput task */
typedef struct _paramtp
{
    char ip_addr[16];           /* ip address */
    short  port;              /* port number */
} paramtpStruct;

#define CMD_TBL_NET    CMD_TBL_ENTRY(          \
	"net",		3,	NULL,       \
	"+net		- Main the net",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_net_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_NET_NETINFO    CMD_TBL_ENTRY(          \
	"netinfo",		7,	cmd_net_netinfo,       \
	"netinfo		- Network information",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_NET_DHCPSINFO  CMD_TBL_ENTRY(          \
	"dhcpsinfo",		9,	cmd_net_dhcpsinfo,       \
	"dhcpsinfo	- DHCP server information",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_NET_MAC    CMD_TBL_ENTRY(          \
	"mac",		3,	NULL,       \
	"mac		- Mac table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_net_mac_tbl,		cmd_net_tbl			\
),

#define CMD_TBL_NET_WIFI    CMD_TBL_ENTRY(          \
	"wifi",		4,	NULL,       \
	"wifi		- wifi table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_net_wifi_tbl,		cmd_net_tbl			\
),

#endif
