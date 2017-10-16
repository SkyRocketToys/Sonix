#include <FreeRTOS.h>
#include <semphr.h>
#include <libmid_td/td.h>
#include <nonstdlib.h>
#include <task.h>
#include <string.h>
#include "wifi_api.h"
#include "lwip/sockets.h"
#include "lwip/dhcps.h"
#include "lwip/netif.h"
#include "easysetup.h"

#define SIG_OFFSET      0
#define VER_OFFSET      4
#define SSIDLEN_OFFSET  6
#define EASYSETUP_SERV_PORT	33699 
#define SSID_OFFSET     8

extern  struct netif EMAC_if;

Easysetup_Info *esinfo = NULL;
int listenfd = -1;

void SwitchToDeviceModeAndConnect(char *SSID, char *PWD){
	xWifiStackEvent_t xTestEvent;
	ip_addr_t xIPAddr, xNetMask, xGateway;
	dhcps_deinit();
	print_msg_queue("\nSSID : %s PASSWORD %s\n", SSID, PWD);

	/* Set up the network interface. */
	ip_addr_set_zero(&xGateway);
	ip_addr_set_zero(&xIPAddr);
	ip_addr_set_zero(&xNetMask);

	netif_set_ipaddr(&EMAC_if, &xIPAddr);	
	netif_set_netmask(&EMAC_if, &xNetMask);	
	netif_set_gw(&EMAC_if, &xGateway);	

	WiFi_Task_UnInit();
	vTaskDelay(500/portTICK_RATE_MS);
	print_msg_queue("\n\nStart Device mode and send Set AP  message to connect ....\n\n");
	WiFi_Task_Init(NULL, WIFI_RUN_MODE_DEV);

	vTaskDelay(1000/portTICK_RATE_MS);
	xTestEvent.eEventType=eWifiSetAPInfo;
    strcpy(xTestEvent.sAPSSID, SSID);
	strcpy(xTestEvent.sAPPassWd, PWD);

	prvSendEventToWiFiTask(&xTestEvent);

}

#if CONFIG_MIDDLEWARE_ES_TONE_DETECTION
void ToneDetectionVerifyCb(unsigned char *aes_decrypt_data,int enc_len,void*cbarg)
{
    tdes_info *info =(tdes_info *) cbarg;
	char signature[4+1];
	char ver[2+1];
	char ssid_len[2+1];
	int ssidlen = 0;
	char ssid[32];
	char pw_len[2+1];
	int pwlen = 0;
	char passwd[64];
	char bid[4+1];
	char id[320];
	int nbid;
	int mode = 0;
	
	strncpy(id,aes_decrypt_data,strlen(aes_decrypt_data));
	strncpy(signature, id + SIG_OFFSET, 4);
	signature[4] = 0;
	strncpy(ver, id + VER_OFFSET, 2);
	ver[2] = 0;
	strncpy(ssid_len, id + SSIDLEN_OFFSET, 2);
	ssid_len[2] = 0;
	ssidlen = simple_strtol(ssid_len, 0, 10);
	strncpy(ssid, id + SSID_OFFSET, ssidlen);
	ssid[ssidlen] = 0;
	strncpy(pw_len, id + SSID_OFFSET + ssidlen, 2);
    pw_len[2] = 0;
	pwlen = simple_strtol(pw_len, 0, 10);
	strncpy(passwd, id + SSID_OFFSET + ssidlen + 2, pwlen);
	passwd[pwlen] = 0;
	strncpy(bid, id + SSID_OFFSET + ssidlen + 2 + pwlen, 4);
	bid[4] = 0;
	nbid = simple_strtol(bid, 0, 16);
	mode = simple_strtol(id + SSID_OFFSET + ssidlen + 2 + pwlen + 4, 0, 10);
	if(listenfd > -1){
		close(listenfd);
		listenfd = -1;
	}
	SwitchToDeviceModeAndConnect(ssid, passwd);
}
#endif


#if CONFIG_MIDDLEWARE_ES_WIFI
void WifiEsVerifyCb(void*cbarg){
	int connfd=0;
	unsigned long long n=0 ;
	struct sockaddr_in servaddr,cliaddr;
	socklen_t clilen;
	unsigned char ssid[32],pwd[32];

	listenfd=socket(AF_INET,SOCK_STREAM,0);

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(EASYSETUP_SERV_PORT);
	bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

	listen(listenfd,1);

	clilen=sizeof(cliaddr);
	connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);

	memset(ssid, 0, sizeof(ssid));
	print_msg_queue("One device connect...\n\n");
	n = recvfrom(connfd,ssid,sizeof(ssid),0,(struct sockaddr *)&cliaddr,&clilen);
	if(n<0){
		print_msg("recvfrom failed n = %d\n",n);
		goto wifies_fin;
		return -1;
	}
	print_msg_queue("Received SSID : %s\n", ssid);
	memset(pwd, 0, sizeof(pwd));

	n = recvfrom(connfd,pwd,sizeof(pwd),0,(struct sockaddr *)&cliaddr,&clilen);
	if(n<0){
		print_msg_queue("recvfrom failed n = %d\n",n);
		goto wifies_fin;
		return -1;
	}
	print_msg("Received Password : %s\n", pwd);
	SwitchToDeviceModeAndConnect(ssid,pwd);

wifies_fin:
	close(connfd);
	close(listenfd);
	listenfd = -1;
	return 0;
}
#endif

#if CONFIG_MIDDLEWARE_ES_QR
void QrEsVerifyCb(unsigned char *data, unsigned int len, void*cbarg){
	char *delim = ":";
	char * pch;
	unsigned char ssid[32],pwd[32];

	print_msg_queue("Get Data : %s length %d\n", data, len);
	pch = strtok(data,delim);
	if(pch)
		strcpy(ssid,pch);
	pch = strtok(NULL,delim);
	if(pch)
		strcpy(pwd,pch);

	if(listenfd > -1){
		close(listenfd);
		listenfd = -1;
	}
	SwitchToDeviceModeAndConnect(ssid,pwd);
	return;
}
#endif

int cmd_verify_easysetup_start(int argc, char* argv[]){

	int mode = 0;
	
	mode = atoi(argv[1]);
	
	if(esinfo)
		vPortFree(esinfo);

	esinfo = es_info_create();

	esinfo->smode = mode;
#if CONFIG_MIDDLEWARE_ES_WIFI
	esinfo->wcb = WifiEsVerifyCb;
	esinfo->wcbarg = NULL;
#endif
#if CONFIG_MIDDLEWARE_ES_TONE_DETECTION
	esinfo->tdcb = ToneDetectionVerifyCb;
	esinfo->tdcbarg = NULL;
#endif
#if CONFIG_MIDDLEWARE_ES_QR
	esinfo->qrcb = QrEsVerifyCb;
	esinfo->qrcbarg = NULL;
#endif	
	if(!mode){
		print_msg_queue("Start easy setup failed with mode %d...\n",mode );
		return -1;
	}
		

	print_msg_queue("Start easy setup mode = %d...\n",mode );

	start_easysetup(esinfo);
}

