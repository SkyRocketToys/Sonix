#ifndef __WIFI_API_H__
#define __WIFI_API_H__

#define MAX_AP_SCAN_NUM						50

#define QUREY_SUCCESS		1
#define QUERY_FAIL			0

typedef enum _QUERY_OID{
	QID_HW_MAC_ADDR,
	SET_HW_MAC_ADDR,
	SET_HW_ESSID_ADDR,
	SET_HW_CHANNEL,
	QID_HW_CHANNEL,
	SET_BEACON_ON,
	SET_BEACON_OFF,
	SET_BEACON_INTERVAL,
	SET_BEACON_SSID,
	QID_BEACON_SSID,
	SET_HW_RETRY_COUNT,
	SET_SECURITY_WEP128,
	SET_SECURITY_WEP128_DISABLED,
	SET_SECURITY_WEP_DISABLED,
	SET_SECURITY_WEP,
	QID_SECURITY_WEP,
	SET_SECURITY_WPA,
	QID_SECURITY_WPA,
	SET_SECURITY_WPA2,
	QID_SECURITY_WPA2,
	SET_START_AP_SCAN,
	SET_STOP_AP_SCAN,
	QID_GET_AP_SCAN_RESULT,
	SET_STORE_AP_INFO_FLASH,
	SET_SMART_CONFIG_ON,	
	SET_SMART_CONFIG_OFF,
	SET_SMART_CONFIG_RESTART,
	SET_SMART_CONFIG_FAIL,
	SET_TX_RETRY_COUNT,
	SET_TX_PHY_RATE,
	SET_TX_AUTO_FALLBACK,
	QID_GET_RX_STREGTH,
	QID_GET_TX_STATUS,
	SET_POWER_SAVE_OFF,
	SET_POWER_SAVE_ON,
	SET_SEND_DISCONNECT,
	SET_MAC_TX_ONOFF,
	SET_EDCCA_ONOFF,
	SET_EDCCA_ED_TH,
	SET_EDCCA_PERIOD,
	SET_EDCCA_FALSE_CCA_TH,
	SET_EDCCA_BLOCK_CHECK_TH,
	/* Additional Debug Information */
	QID_GET_ENTRY_INFO,	
	SET_AP_GTK_TIME,
	QID_GET_ROUTER_INFO,
	QID_GET_SELF_INFO,
	SET_PACKET_TYPE_PROTECT,
	QID_AUTH_MODE,
	QID_RUN_MODE,
	SET_TX_CHANNEL_POWER
} QUERY_OID;


typedef enum
{
   	eWifitest 		= 0,				/* For Test */
	eWifiInit,							/* For WIFI Initial */
	eWifiConnect,						/* For Starting to connect AP */
	eWifiDisconnect,					/* For Disconnecting Process and stay idling */
	eWifiSetAPInfo						/* For Setting AP Info to WIFI Module */
} eWifiEvent_t;

/* Notify events for Apps */
typedef enum
{
	eWifiInitDone	= 0,				/* Wifi initial done (AP/Station mode) */
	eWifiScanDone,						/* Wifi scan router done (AP mode) */
	eWifiGetAPDone,						/* Wifi check target router done (Station mode) */
	eWifiTestClose						/* Test event to trigger restart ipc flow */
} eWifiNotifyEvent;

typedef enum
{
	 SET_AP_SSID_PSWD,
	 SET_SCAN_ALL_AP,
	 SEND_PROBE_RESPOND,
	 SEND_AUTH_RESPOND,
	 SEND_ASSOCIATION_RESPOND,	 
} WIFI_AP_MESSAGE;

/* Wifi operation mode */
typedef enum _WIFI_RUN_MODE {
	 WIFI_RUN_MODE_IDLE 	= 1,
	 WIFI_RUN_MODE_AP 		= 2,
	 WIFI_RUN_MODE_DEV 		= 3,
} WIFI_RUN_MODE;

typedef enum _Auth_type {
	AUTH_NONE = 0,
	AUTH_WEP,
	AUTH_WPA,
	AUTH_WPA2,
	AUTH_UNKNOWN
} enc_auth_t;

typedef struct Wifi_TASK_COMMANDS
{
	eWifiEvent_t 	eEventType;			/* Message ID */
	char			sAPSSID[32];		/* AP SSID to be set to WiFi Module	*/	
	char 			sAPPassWd[32];		/* AP Password to be set to WiFi Module */
} xWifiStackEvent_t;


typedef struct Wifi_AP_MESSAGE_COMMAND
{
	 char 			message; 			/* Message ID */
	 char			sAPSSID[32];		/* AP SSID to be set to WiFi Module	 */		 
	 char 			sAPPassWd[32];		/* AP Password to be set to WiFi Module */
	 unsigned char 	MgmAdd[6];			/* Response Addr */
	 unsigned int	reserved;			/* status code */
} AP_Message, *pAP_Message;


typedef struct _APScanResultElement 
{	
	unsigned char 	channel;
	unsigned char 	SsIdLen;
	unsigned char 	SsId[32];
	unsigned char	BssId[6];
	char 			auth[16]; 			/* for APP storead */
	unsigned char 	auth_mode;
	unsigned char 	uenc_mode;			/* unicast encrypt mode */
	unsigned char	menc_mode;			/* multicast encrypt mode (In WPA/WPA2) */
	unsigned char 	signal;
	struct _APScanResultElement *NextElem;
} APScanResultElement, *PAPScanResultElement;


typedef void (*wifi_event_rsp_hndler)(eWifiNotifyEvent event);

/* Initial and Uninitial */
void WiFi_Task_Init(wifi_event_rsp_hndler hndler, unsigned char mode);
void WiFi_Task_UnInit(void);

/* Control interface */
unsigned int WiFi_QueryAndSet(unsigned char item, unsigned char *pOutBuf, unsigned short *pLen);
int prvSendEventToWiFiTask(xWifiStackEvent_t *xEventMessage);

/* Additional interface for querying information */
unsigned char * wlan_get_get_mac_addr(void);
PAPScanResultElement WiFi_get_scan_RouterInfo(void);
unsigned char WiFiAPLinkNum(void);

/* direct interface for lwip to transmit packet */
int wifi_mac_pkt_send(void *packet, int length);
unsigned char WIFI_AP_Initial_Done(void);


#endif  /*__WIFI_API_H__ */ 
