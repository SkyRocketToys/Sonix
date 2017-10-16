
#ifndef _CMD_QR_SCAN_H_
#define _CMD_QR_SCAN_H_


int cmd_qr_scan_decode(int argc, char* argv[]);



#define CMD_TBL_QR_SCAN		CMD_TBL_ENTRY(		\
	"qrscan",	6,      NULL,			\
	"+qrscan		- qrscan command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_qr_scan_tbl,		cmd_verify_tbl			\
),

#define CMD_TBL_QR_SCAN_DECODE	CMD_TBL_ENTRY(		\
	"decode",		6,      cmd_qr_scan_decode,			\
	"decode		- QR SCAN command decode\n \
			example: decode", CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),


#endif
