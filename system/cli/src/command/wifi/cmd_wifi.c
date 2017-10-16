#include <FreeRTOS.h>
#include <stdio.h>
#include "cmd_gpio.h"
#include "cmd_debug.h"
#include "printlog.h"
#include <nonstdlib.h>
#include <gpio/gpio.h>
#include <string.h>
#include <nonstdlib.h>
#include <queue.h>
//#include <wifi/WiFiquery.h>
//#include <wifi/WiFi.h>
#include <wifi/wifi_api.h>

#define printf print_msg_queue


char *  EncName[] = {"OPEN-NONE","OPEN-WEP64","WPA", "WPA2", "OPEN-WEP128"};

#define ENC_NONE		0
#define ENC_WEP64		1
#define ENC_TKIP		2
#define ENC_CCMP		3
#define ENC_WEP128		4
#define ENC_UNKONWN		5


void help(char *FuncString)
{
	printf(" Usage: %s get/set [param]\n", FuncString);
	printf("	[param]: auth, (only get can be used)\n");
	printf("	[param]: ssid, (if set, need input specified ssid (length up to limit 32)\n");
	printf("	[param]: pswd, (if set, need input specified password\n");
	printf("	[param]: chnl, (if set, need input specified channel\n");
	printf("  Ex: ap_ctrl get auth\n");
	printf("  Ex: ap_ctrl set ssid test123\n");
	printf(" ======================================================================================\n");
}


void sta_help(char *FuncString)
{
	printf(" Usage: %s [Auth] [SSID] [Password]\n", FuncString);
	printf("	[Auth]: None(0), WEP(1), WPA(2), WPA2(2)\n");
	printf("	[SSID]: specified target ssid\n");
	printf("	[Password]: specified target password\n");
	printf(" Ex: sta_set 2 TESTAP-WPA 12345678\n");
	printf(" Ex: sta_set 1 TESTAP-WEP 12345\n");
	printf(" ======================================================================================\n");
}


int cmd_wifi_ap_ctrl(int argc, char * argv[])
{
	int enc = 0, channel = 0, run_mode = 0;
	int len_ssid = 0, len_password = 0;
	char UserSpecifiedSSID[32] = {0};
	char UserSpecifiedPASSWORD[64] = {0};
		
	if (argc < 2 || argc > 4) {
		goto error_hndl;
	}

	WiFi_QueryAndSet(QID_RUN_MODE, (unsigned char *)&run_mode, 1);

	if (run_mode != WIFI_RUN_MODE_AP)
	{
		/* Unint Wifi */
		printf("Restart wifi to AP moden\n");
		WiFi_Task_UnInit();
		vTaskDelay(1000);
		WiFi_Task_Init(NULL, WIFI_RUN_MODE_AP);
		vTaskDelay(500 / portTICK_RATE_MS);
	}
	
	WiFi_QueryAndSet(QID_AUTH_MODE, (unsigned char *)&enc, 1);

	if (enc == ENC_UNKONWN) {
		printf("Cannot extract current AP mode enth mothod!!\n");
		goto error_hndl;
	}

	if (strncasecmp(argv[1], "get", 3) == 0) {
		if (strncasecmp(argv[2], "auth", 4) == 0) {
			printf("Current enc mode = %s\n", EncName[enc]);
		}
		else if (strncasecmp(argv[2], "ssid", 4) == 0) {
			WiFi_QueryAndSet(QID_BEACON_SSID, (unsigned char *)&UserSpecifiedSSID, (unsigned short *)&len_ssid);
			printf("Current SSID mode = %s\n", UserSpecifiedSSID);
		}
		else if (strncasecmp(argv[2], "pswd", 4) == 0) {
			if (enc == ENC_NONE)
				printf("pswd is open without password!\n");
			else if (enc == ENC_WEP64 || enc == ENC_WEP128) {
				WiFi_QueryAndSet(QID_SECURITY_WEP, (unsigned char *)&UserSpecifiedPASSWORD, (unsigned short *)&len_password);
				printf("Current Password mode = %s\n", UserSpecifiedPASSWORD);
			}
			else if (enc == ENC_TKIP || enc == ENC_CCMP) {
				WiFi_QueryAndSet(QID_SECURITY_WPA, (unsigned char *)&UserSpecifiedPASSWORD, (unsigned short *)&len_password);
				printf("Current Password mode = %s\n", UserSpecifiedPASSWORD);
			}
			else {
				printf("unknown error (enc mode = %d!!\n", enc);
				goto error_hndl;
			}
		}
		else if (strncasecmp(argv[2], "chnl", 4) == 0) {
			WiFi_QueryAndSet(QID_HW_CHANNEL, (unsigned char *)&channel, 1);
			printf("Current channel = %d\n", channel);
		}
		else
			goto error_hndl;
	}
	else if (strncasecmp(argv[1], "set", 3) == 0) {
		if (argc < 3)
			goto error_hndl;
		
		if (strncasecmp(argv[2], "ssid", 4) == 0) {
			len_ssid = strlen(argv[3]);
			if (len_ssid > 32) {
				printf("Over ssid limit %s:(%d) > 32\n", argv[3], len_ssid);
				goto error_hndl;
			}
			memcpy(UserSpecifiedSSID, argv[3], len_ssid);

			printf("Set ssid(%s):(%d)\n", UserSpecifiedSSID, len_ssid);

			WiFi_QueryAndSet(SET_BEACON_OFF, NULL, NULL);
			WiFi_QueryAndSet(SET_BEACON_SSID, (unsigned char *)UserSpecifiedSSID, (unsigned short *)&len_ssid);
			WiFi_QueryAndSet(SET_BEACON_ON, NULL, NULL);
		}
		else if (strncasecmp(argv[2], "pswd", 4) == 0) {
			len_password = strlen(argv[3]);

			/* check password and its correspond auth check */
			if (enc == ENC_NONE) {
				printf("In open mode, won't set password\n");
			}
			else if (enc == ENC_WEP64) {
				if (len_password != 5)
				 goto error_hndl;
				
				memcpy(UserSpecifiedPASSWORD, argv[3], len_password);
				WiFi_QueryAndSet(SET_BEACON_OFF, NULL, NULL);
				WiFi_QueryAndSet(SET_SECURITY_WEP, (unsigned char *)UserSpecifiedPASSWORD, (unsigned short *)&len_password);
				WiFi_QueryAndSet(SET_BEACON_ON, NULL, NULL);
			}
			else if (enc == ENC_WEP128) {
				memcpy(UserSpecifiedPASSWORD, argv[3], len_password);
				WiFi_QueryAndSet(SET_BEACON_OFF, NULL, NULL);
				WiFi_QueryAndSet(SET_SECURITY_WEP128, (unsigned char *)UserSpecifiedPASSWORD, (unsigned short *)&len_password);
				WiFi_QueryAndSet(SET_BEACON_ON, NULL, NULL);

			}
			else if (enc == ENC_TKIP || enc == ENC_CCMP){
				memcpy(UserSpecifiedPASSWORD, argv[3], len_password);
				WiFi_QueryAndSet(SET_BEACON_OFF, NULL, NULL);
				WiFi_QueryAndSet((enc == ENC_TKIP)?(SET_SECURITY_WPA):(SET_SECURITY_WPA2), (unsigned char *)UserSpecifiedPASSWORD, (unsigned short *)&len_password);
				WiFi_QueryAndSet(SET_BEACON_ON, NULL, NULL);
			}
			else
				goto error_hndl;
		}
		else if (strncasecmp(argv[2], "chnl", 4) == 0) {
			/* check channel legal or not */
			channel = atoi(argv[3]);

			if (channel < 1 || channel > 15)
				goto error_hndl;
			
			WiFi_QueryAndSet(SET_BEACON_OFF, NULL, NULL);
			WiFi_QueryAndSet(SET_HW_CHANNEL, (unsigned char *)&channel, 1);
			WiFi_QueryAndSet(SET_BEACON_ON, NULL, NULL);
		}
		else
			goto error_hndl;
	}
	else {
		printf("invalid ctrl %s\n", argv[1]);
		goto error_hndl;
	}
	
	return pdPASS;

error_hndl:
	help("ap_ctrl");
	return pdFAIL;

}

int cmd_wifi_sta_set(int argc, char * argv[])
{
	int Authmode = -1;
	int len_ssid = 0, len_password = 0;
	char UserSpecifiedSSID[32] = {0};
	char UserSpecifiedPASSWORD[64] = {0};
	xWifiStackEvent_t xTestEvent;
		
	if (argc < 3) {
		goto error_hndl;
	}

	Authmode = simple_strtoul(argv[1], NULL, 10);
	
	if (Authmode < AUTH_NONE || Authmode > AUTH_WPA2) {
		printf("Invalid Auth mode = %d\n", Authmode);
		goto error_hndl;
	}
	
	len_ssid = strlen(argv[2]);
	if (len_ssid > 32) {
		printf("Len SSID(%d) is invalid (Upto max:32)\n", len_ssid);
		goto error_hndl;
	}
	memcpy(UserSpecifiedSSID, argv[2], len_ssid);
	
	if (Authmode != AUTH_NONE && argc < 4)
	{
		printf("Auth mode isn't None, need password...\n");
		goto error_hndl;
	}

	len_password = strlen(argv[3]);

	/* check Password valid */
	if (Authmode == AUTH_WEP) 
	{
		if (len_password != 5 || len_password != 13) 
		{
			printf("Password(%s:%d) must satisfy WEP-64 (len = 5), WEP-128 (len = 13)\n", argv[3], len_password);
			goto error_hndl;
		}
	}
	else if ((Authmode == AUTH_WPA) || (Authmode == AUTH_WPA2))
	{
		if (len_password < 8) 
		{
			printf("Password(%s:%d) must >= 8\n", argv[3], len_password);
			goto error_hndl;
		}
	}
	else 
	{
		/* AUTH_NONE */

	}

	memcpy(UserSpecifiedPASSWORD, argv[3], len_password);

	printf("[%s] Auth(%s), SSID(%s), PASSWORD(%s)\n", __FUNCTION__, EncName[Authmode], UserSpecifiedSSID, UserSpecifiedPASSWORD);

	/* Unint Wifi */
	WiFi_Task_UnInit();
	vTaskDelay(1000);

	WiFi_Task_Init(NULL, WIFI_RUN_MODE_DEV);
	vTaskDelay(1000);

	/* Setup Wifi Sta setting */
	memset(&xTestEvent, 0x0, sizeof(xTestEvent));
	print_msg("\n eWifiSetAPInfo .....\n.");
	xTestEvent.eEventType = eWifiSetAPInfo;
	strcpy((char *)xTestEvent.sAPSSID, UserSpecifiedSSID);
	strcpy((char *)xTestEvent.sAPPassWd, UserSpecifiedPASSWORD);
	prvSendEventToWiFiTask(&xTestEvent);
	
	return pdPASS;
	
error_hndl:
	sta_help("sta_set");
	return pdFAIL;

}


#define	G_54M		0x47
#define	G_48M		0x46
#define	G_36M		0x45
#define	G_24M		0x44
#define	G_18M		0x43
#define	G_12M		0x42
#define	G_9M		0x41
#define	G_6M		0x40

#define	B_11M		0x03
#define	B_55M		0x02
#define	B_2M		0x01
#define	B_1M		0x00


#define TX_FALLBACK_ON		0
#define TX_FALLBACK_OFF		1


/* Note: It only change transmit rate not display mode */
int cmd_wifi_ctrl(int argc, char * argv[])
{
	int num = -1,mode = 0, val = 1;

	unsigned char phy_rate = 0;
	unsigned char auto_fallback = 0;
	unsigned char ch;

	if (argc < 3) {
		printf(" Usage: wifi_ctrl [ch] [mode] [value]\n");
		printf(" num: 1~13\n");
		printf(" mode: g , b\n");
		printf(" value: 54/48/36/24/18/12/9/6 for g\n");
		printf(" value: 11/55/2/1 for b\n");
		return pdFAIL;
	}

	num = simple_strtoul(argv[1], NULL, 10);
	val = simple_strtoul(argv[3], NULL, 10);

	if ((num > 0)&&(num < 14)){
		ch = (unsigned char) num;

	} else {
		printf("%s:%d = not support only ch 1~13\n",__func__,__LINE__);
		return pdFAIL;
	}


	if(strcmp(argv[2], "g") == 0) {
		switch(val)
		{
			case 0x36: //54
			phy_rate = G_54M;
			break;

			case 0x30: //48
			phy_rate = G_48M;
			break;

			case 0x24: //36
			phy_rate = G_36M;
			break;

			case 0x18: //24
			phy_rate = G_24M;
			break;

			case 0x12: //18
			phy_rate = G_18M;
			break;

			case 0x0c: //12
			phy_rate = G_12M;
			break;

			case 0x09: //9
			phy_rate = G_9M;
			break;

			case 0x06: //6
			phy_rate = G_6M;
			break;

			default:
			printf("%s:%d = not support only g54/48/36/24/18/12/9/6 , b11/55/2/1\n",__func__,__LINE__);
			return pdFAIL;
		}
	}
	else if(strcmp(argv[2], "b") == 0)	{
		switch(val)
		{
			case 0x0b: //11
			phy_rate = B_11M;
			break;

			case 0x37: //55
			phy_rate = B_55M;
			break;

			case 0x02: //2
			phy_rate = B_2M;
			break;

			case 0x01: //1
			phy_rate = B_1M;
			break;

			default:
			printf("%s:%d = not support only g54/48/36/24/18/12/9/6 , b11/55/2/1\n",__func__,__LINE__);
			return pdFAIL;
		}
	}
	else{
		printf("%s:%d = not support only g54/48/36/24/18/12/9/6 , b11/55/2/1\n",__func__,__LINE__);
		return pdFAIL;
	}

	 WiFi_QueryAndSet(SET_HW_CHANNEL, &ch, NULL);
	 WiFi_QueryAndSet(SET_TX_PHY_RATE, &phy_rate, NULL);

	 auto_fallback = TX_FALLBACK_OFF;
	 WiFi_QueryAndSet(SET_TX_AUTO_FALLBACK, &auto_fallback, NULL);
	
	return pdPASS;	 
}

void info_help(void)
{
	printf(" Usage: %s [Action] [Param]\n", "cmd_wifi_info");
	printf("	Action: 1: Get All entry information\n");
	printf("            2: Set AP GTK update time (Param)\n");
	printf("            3: Get Router information (Station mode)\n");
	printf("			4: Get Self information (AP mode)\n");

}

typedef enum _Info_type {
	/* AP mode */
	LIST_ALL_ENTRY_INFO = 1,
	SET_GTK_TIME,
	/* Station mode */
	LIST_ROUTER_INFO,
	/* Both */
	LIST_SELF_INFO,
	SET_TRY_EZ
} WIFI_INFO_TYPE;

int cmd_wifi_info(int argc, char * argv[])
{
	int opt = 0;
	if (argc < 2) {
		goto error_hndl;
	}

	opt = simple_strtoul(argv[1], NULL, 10);

	if (opt < LIST_ALL_ENTRY_INFO || opt > SET_TRY_EZ) {
		printf("Invalid opt (%d)\n", opt);
		goto error_hndl;
	}
	
	switch (opt)
	{
		case LIST_ALL_ENTRY_INFO:
			WiFi_QueryAndSet(QID_GET_ENTRY_INFO, NULL, NULL);
			break;

		case SET_GTK_TIME:
			{
				int time = simple_strtoul(argv[2], NULL, 10);
				WiFi_QueryAndSet(SET_AP_GTK_TIME, NULL, (unsigned short *)&time);
			}
			break;

		case LIST_ROUTER_INFO:
			WiFi_QueryAndSet(QID_GET_ROUTER_INFO, NULL, NULL);
			break;

		case LIST_SELF_INFO:
			WiFi_QueryAndSet(QID_GET_SELF_INFO, NULL, NULL);
			break;
			
		default:
			break;
	}
	
	return pdPASS;
	
error_hndl:
	info_help();
	return pdFAIL;
}
