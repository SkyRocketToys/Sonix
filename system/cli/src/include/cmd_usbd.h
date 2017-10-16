#ifndef _CMD_USBD_H_
#define _CMD_USBD_H_

int cmd_usbd_class_mode(int argc, char* argv[]);


#define CMD_TBL_USBD		CMD_TBL_ENTRY(		\
	"usbd",		4,      NULL,			\
	"+usbd		- USB Device Class command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_usbd_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_USBD_CLASS_MODE	CMD_TBL_ENTRY(		\
	"mode",			4,      cmd_usbd_class_mode,	\
	"mode		- Change Current USB Class Mode",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),


#endif /* _CMD_USBD_H_ */
