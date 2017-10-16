/**
 * @file
 * A simple demo application.
 *
 * @author Betters Tsai
 */


#include <stddef.h>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <bsp.h>
#include <nonstdlib.h>
#include <libsnx_cloud/snx_cloud.h>
#include "media.h"
#include "snx_json_test.h"

#define TEST_TIMER 0

snx_streamer_t *psnx_streamer;

#if TEST_TIMER
TimerHandle_t pxTimer1, pxTimer2, pxTimer3;
#endif

xTaskHandle xTimeHndl;

enum SONiX_Cloud_Stat {
	eSTART = 0,
	eSTOP
};

enum SONiX_Cloud_Stat sonix_cloud_stat;

#if TEST_TIMER
void vTimerCallback_1(TimerHandle_t pxTimer)
{
	print_msg("Here is vTimerCallback_1\n");
	if (sonix_cloud_stat == eSTOP) {
		snx_streamer_start(psnx_streamer);
		sonix_cloud_stat = eSTART;
	}
	set_timer_service_active(pxTimer2);
	return;
}


void vTimerCallback_2(TimerHandle_t pxTimer)
{
	print_msg("Here is vTimerCallback_2\n");
	if (sonix_cloud_stat == eSTART) {
		snx_streamer_stop(psnx_streamer);
		sonix_cloud_stat = eSTOP;
	}
	set_timer_service_active(pxTimer1);
	return;
}

void vTimerCallback_3(TimerHandle_t pxTimer)
{
	print_msg("Here is vTimerCallback_3\n");
	set_timer_service_active(pxTimer3);
	return;
}

int set_timer_service_active(TimerHandle_t pxTimer)
{
	if (pxTimer == NULL)
		return -1;
	else
	{
		if (xTimerStart(pxTimer, 0) != pdPASS)
		{
			print_msg("Timer service is failed to be active");
			return -1;
		}
	}
	return 0;
}
#endif

/*
void snx_streamer_stat_cb(snx_streamer_t *pstreamer, void *cbarg)
{
	static int time_flag_sync = 0;
	static int flag_play_audio_online_pcm = 0;

	switch (event->type) {
		case SNX_ONLINE:

			break;

		case SNX_OFFLINE:

			break;
		case SNX_CONNECTING:
			switch (event->connecting.retry_count) {
				case 1:
					printf("connecting to master server...\n");
					break;
				case 2:
					printf("retrying for the 1st time\n");
					break;
				case 3:
					printf("retrying for the 2nd times\n");
					break;
				case 4:
					printf("retrying for the 3rd times\n");
					break;
				default:
					printf("retrying for the %dth times\n", event->connecting.retry_count);
					break;
			}
			break;

			case SNX_CONNECTING_MSG:
				printf("connecting to message server\n");
				break;
			case SNX_RECONNECTING:
				printf("reconnecting in %d secs\n", event->reconnecting.count_down);
				break;
			default:
				printf("WARNING: receive unknown event %d \n",event->type);
				break;
	}
}
*/
/*---------------------------------------------------------------------------------*/

void snx_lib_binding_errcb(int result)
{
	if (result < 0) {
		print_msg("Binding fail\n");
	}
	else {
		print_msg("Binding success\n");
		
	}
}

void snx_lib_cloud_test_main(void)
{
	//print_msg("%s\n", PRODUCT_KEY);

	psnx_streamer = snx_streamer_new(SERVER_URL, PROXY_URL, MODEL, BRAND,
			PRODUCT_KEY, FW_VERSION, QID, USE_ENCRYPT, USE_P2P_NAT, NULL, NULL);

	sonix_cloud_stat = eSTOP;

	if (!psnx_streamer) {
		print_msg("Create SONiX streamer failed !!");
	}

	/* Setup */
	/* snx_cloud_initial_setup(psnx_streamer, LOCAL_MIN_PORT, LOCAL_MAX_PORT); * need modified zbl.*/
	snx_cloud_set_heartbeat_interval(psnx_streamer, HEARTBEAT_INTERVAL);
	snx_cloud_set_relay_timeout(psnx_streamer, RELAY_TO);
	snx_cloud_set_maxima_user(psnx_streamer, MAXIMA_USER);
	snx_cloud_set_dev_fw_stat(psnx_streamer, DEV_IN_NORMAL);

	/* AV initialization */
	snx_AVMedia_Init(psnx_streamer, NULL, NULL);

#if TEST_TIMER
	/* Setup Timer */
	pxTimer1 = xTimerCreate("Timer1", (5000/portTICK_RATE_MS), pdFAIL, NULL, vTimerCallback_1);
	if (!pxTimer1)
	{
		print_msg("Create timer object 1 faled\n");
	}

	pxTimer2 = xTimerCreate("Timer1", (5000/portTICK_RATE_MS), pdFAIL, NULL, vTimerCallback_2);
	if (!pxTimer2)
	{
		print_msg("Create timer object 1 faled\n");
	}

	pxTimer3 = xTimerCreate("Timer1", (1000/portTICK_RATE_MS), pdFAIL, NULL, vTimerCallback_3);
	if (!pxTimer2)
	{
		print_msg("Create timer object 1 faled\n");
	}

	//set_timer_service_active(pxTimer1);
	//set_timer_service_active(pxTimer3);
#endif

	snx_set_binding(psnx_streamer, 0, MAXIMA_BIND_CHECK, snx_lib_binding_errcb);

	print_msg("Wakeup SONiX streamer\n");
	if (snx_streamer_start(psnx_streamer)) {
		print_msg("startup SONiX streamer fail\n");
	}

	//snx_json_test_main();
}
/*---------------------------------------------------------------------------------*/
