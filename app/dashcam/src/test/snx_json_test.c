#include <stddef.h>

#include <FreeRTOS.h>
#include <bsp.h>
#include <nonstdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "snx_json_test.h"

/* Predefine  for json example */
#define NAMESPACE 		"media"
#define VERSION			(1)
#define CONTROL			"play"
#define H_SESSION		"bcdd4"
#define AUTH			"ZM8666698VV:8"
#define MSGSRVNAME		"alimessaging"
#define MSGUID			"2695133f-3887-49e8-a839-d54786cb1cb5"
#define CLIENT			"bad3ea5e-d020-4ad6-9603-713a0fadaec7"
#define PROFILE			"cloud"
#define B_SESSION		"570532594"
#define ADDRESS			"192.168.10.102"
#define TRACK_V			"video"
#define TRACK_A			"audio"
#define TRACK_T			"talk"
#define VIDEO_DATA_PORT	(6001)
#define VIDEO_REPO_PORT	(6002)
#define AUDIO_DATA_PORT	(6003)
#define AUDIO_REPO_PORT	(6004)
#define TALK_DATA_PORT	(6005)
#define TALK_REPO_PORT	(6006)
#define TIMEOUT			(10)
#define APP_TYPE		"android"
#define TOKEN			"2695133f-3887-49e8-a839-d54786cb1cb5"
#define TOKEN_TO		"be5dc98e-4dcd-47de-aa05-cfcfc6cf7e10"
#define AUID			"a73a1ca9-7830-43a2-8228-27b3cc9678fd"


#define USE_JSON_EXSAMPLE_1 (1)

/* global value */
snx_json_t *test_root;
snx_json_t *test_dep_root;
snx_jtoken_t *tokener;

/* static function declartion */
static snx_json_t *test_create_test_sample(char **output);
static snx_json_t *test_create_test_sample2(char **output);
static int test_decomp_test_smaple(snx_json_t *root_obj);
static int test_decomp_test_smaple2(snx_json_t *root_obj);


/* JSON examaple main */
int snx_json_test_main()
{
	char *test_output_string;


	if(!(tokener = snx_json_token_new()))
	{
		print_msg("Couldn't create test tokener\n");
		return -1;
	}
#if USE_JSON_EXSAMPLE_1
	if (!(test_root = test_create_test_sample(&test_output_string)))
	{
		print_msg("Couldn't create test root json object\n");
		return -1;
	}
#else
	if (!(test_root = test_create_test_sample2(&test_output_string)))
	{
		print_msg("Couldn't create test root json object\n");
		return -1;
	}
#endif

#if USE_JSON_EXSAMPLE_1
	print_msg("[%d]%s\n", strlen(test_output_string), test_output_string);
#endif

	snx_json_tokener_reset(tokener);

	if (!(test_dep_root =  snx_json_token_parse(tokener, test_output_string, strlen(test_output_string))))
	{
		print_msg("Couldn't create test parse json object\n");
		return -1;
	}

#if USE_JSON_EXSAMPLE_1
	print_msg("========================= Original JSON ROOT ========================= \n");
	test_decomp_test_smaple(test_root);
	print_msg("========================= JSON PARSE ROOT ========================= \n");
	test_decomp_test_smaple(test_dep_root);
#else
	print_msg("========================= Original JSON ROOT ========================= \n");
	test_decomp_test_smaple2(test_root);
	print_msg("========================= JSON PARSE ROOT ========================= \n");
	test_decomp_test_smaple2(test_dep_root);
#endif

	/* release jsob related object */

	snx_json_tokener_free(tokener);
	snx_json_obj_free(test_dep_root);
	snx_json_obj_free(test_root);
	free(test_output_string);
	return 0;
}

static snx_json_t *test_create_test_sample(char **output)
{
	int rc = 0;
	int i = 0;
	snx_json_t *root,  *jobj_arr_int, *jobj_arr_str;
	uint8_t output_buf[128];
	//char *output_string;

	if (!(root = snx_json_obj_new()))
	{
		print_msg("Couldn't create JSON obj \n");
		return NULL;
	}

	if ((rc = snx_json_obj_add_int(root, "Heartbeat", 20)) != 0)
	{
		print_msg("Couldn't add JSON integer obj \n");
		snx_json_obj_free(root);
		return NULL;
	}

	if ((rc = snx_json_obj_add_str(root, "Message Server", "htt://dev.test.msgsrv.com.tw")) != 0)
	{
		print_msg("Couldn't add JSON string obj \n");
		snx_json_obj_free(root);
		return NULL;
	}

	if ((rc = snx_json_obj_add_array(root, "Test Array integer", &jobj_arr_int)) != 0)
	{
		print_msg("Couldn't add JSON string obj \n");
		snx_json_obj_free(root);
		return NULL;
	}

	for (i = 0; i < 4; i++)
	{
		if ((rc = snx_json_obj_add_array_int(jobj_arr_int, i*10)) != 0)
		{
			print_msg("Couldn't add JSON int obj \n");
			snx_json_obj_free(jobj_arr_int);
			snx_json_obj_free(root);

			return NULL;
		}
	}

	if ((rc = snx_json_obj_add_array(root, "Test array string", &jobj_arr_str)) != 0)
	{
		print_msg("Couldn't add JSON string obj \n");
		snx_json_obj_free(jobj_arr_int);
		snx_json_obj_free(root);
		return NULL;
	}

	for (i = 0; i < 4; i++)
	{
		memset(output_buf, 0x0, sizeof(output_buf));
		snprintf((char *)output_buf, sizeof(output_buf), "test-%d", i);

		if ((rc = snx_json_obj_add_array_str(jobj_arr_str, (char *)output_buf)) != 0)
		{
			print_msg("Couldn't add JSON string obj \n");
			snx_json_obj_free(jobj_arr_int);
			snx_json_obj_free(jobj_arr_str);
			snx_json_obj_free(root);
			return NULL;
		}
	}

	*output = snx_json_object_to_json_string(root);

	return root;
}

static snx_json_t *test_create_test_sample2(char **output)
{
	int rc = 0;
	int i, num_track = 3;
	snx_json_t *root,  *jobj_header, *jobj_reply, *jobj_message, *jobj_arr_address, *jobj_track, *jobj_inter_track, *jobj_port, *jobj_pport;


	if (!(root = snx_json_obj_new()))
	{
		print_msg("Couldn't create JSON obj \n");
		return NULL;
	}

	if ((rc = snx_json_obj_add_str(root, "namespace", NAMESPACE)) != 0)
	{
		print_msg("Couldn't add JSON string obj \n");
		return NULL;
	}

	/* ---------------------- header part ---------------------- */
	if ((rc = snx_json_obj_add_obj(root, "header", &jobj_header)) != 0)
	{
		print_msg("Couldn't new header obj \n");
		return NULL;
	}

	if ((rc = snx_json_obj_add_int(jobj_header, "version", VERSION))) {
		print_msg("failed to add version\n");
		return NULL;
	}

	if ((rc = snx_json_obj_add_str(jobj_header, "name", CONTROL))) {
		print_msg("failed to add name\n");
		return NULL;
	}

	if ((rc = snx_json_obj_add_str(jobj_header, "session", H_SESSION))) {
		print_msg("failed to add session\n");
		return NULL;
	}

	if ((rc = snx_json_obj_add_str(jobj_header, "auth", AUTH))) {
		print_msg("failed to add session\n");
		return NULL;
	}

	if ((rc = snx_json_obj_add_obj(jobj_header, "reply", &jobj_reply)) != 0)
	{
		print_msg("Couldn't new reply obj \n");
		snx_json_obj_free(root);
		return NULL;
	}

	if ((rc = snx_json_obj_add_str(jobj_reply, "server", MSGSRVNAME))) {
		print_msg("failed to add session\n");
		return NULL;
	}

	if ((rc = snx_json_obj_add_str(jobj_reply, "uid", MSGUID))) {
		print_msg("failed to add session\n");
		return NULL;
	}

	/* ---------------------- message part ---------------------- */
	if ((rc = snx_json_obj_add_obj(root, "message", &jobj_message)) != 0)
	{
		print_msg("Couldn't new message obj \n");
		snx_json_obj_free(root);
		return NULL;
	}

	if ((rc = snx_json_obj_add_str(jobj_message, "client", CLIENT))) {
		print_msg("failed to add client\n");
		return NULL;
	}

	if ((rc = snx_json_obj_add_str(jobj_message, "profile", PROFILE))) {
		print_msg("failed to add profile\n");
		return NULL;
	}

	if ((rc = snx_json_obj_add_array(jobj_message, "addresses", &jobj_arr_address)) != 0)
	{
		print_msg("Couldn't add JSON address obj \n");
		return NULL;
	}

	if ((rc = snx_json_obj_add_array_str(jobj_arr_address, ADDRESS)) != 0)
	{
		print_msg("Couldn't add address string obj \n");
		return NULL;
	}

	if ((rc = snx_json_obj_add_array(jobj_message, "tracks", &jobj_track)) != 0)
	{
		print_msg("Couldn't add JSON address obj \n");
		return NULL;
	}

	for (i = 0; i < num_track; i++)
	{
		const char* track_name = (i == 0)?(TRACK_V):((i==1)?(TRACK_A):(TRACK_T));
		uint16_t data_p = (i == 0)?(VIDEO_DATA_PORT):((i==1)?(AUDIO_DATA_PORT):(TALK_DATA_PORT));
		uint16_t repo_p = (i == 0)?(VIDEO_REPO_PORT):((i==1)?(AUDIO_REPO_PORT):(TALK_REPO_PORT));

		if ((rc = snx_json_obj_add_array_obj(jobj_track, &jobj_inter_track)) != 0)
		{
			print_msg("Couldn't add JSON obj into array\n");
			return NULL;
		}

		if ((rc = snx_json_obj_add_str(jobj_inter_track, "name", track_name))) {
			print_msg("failed to add profile\n");
			return NULL;
		}

		if ((rc = snx_json_obj_add_array(jobj_inter_track, "port", &jobj_port)) != 0)
		{
			print_msg("Couldn't add JSON port obj \n");
			return NULL;
		}

		if ((rc = snx_json_obj_add_array_int(jobj_port, data_p)) != 0)
		{
			print_msg("Couldn't add address string obj \n");
			return NULL;
		}

		if ((rc = snx_json_obj_add_array_int(jobj_port, repo_p)) != 0)
		{
			print_msg("Couldn't add address string obj \n");
			return NULL;
		}

		if ((rc = snx_json_obj_add_array(jobj_inter_track, "public port", &jobj_pport)) != 0)
		{
			print_msg("Couldn't add JSON port obj \n");
			return NULL;
		}

		if ((rc = snx_json_obj_add_array_int(jobj_pport, data_p)) != 0)
		{
			print_msg("Couldn't add address string obj \n");
			return NULL;
		}

		if ((rc = snx_json_obj_add_array_int(jobj_pport, repo_p)) != 0)
		{
			print_msg("Couldn't add address string obj \n");
			return NULL;
		}

		if ((rc = snx_json_obj_add_int(jobj_inter_track, "timeout", TIMEOUT))) {
			print_msg("failed to add profile\n");
			return NULL;
		}

	}

	if ((rc = snx_json_obj_add_str(root, "type", APP_TYPE))) {
		print_msg("failed to add profile\n");
		return NULL;
	}

	if ((rc = snx_json_obj_add_str(root, "toekn", TOKEN))) {
		print_msg("failed to add profile\n");
		return NULL;
	}

	if ((rc = snx_json_obj_add_str(root, "toeknto", TOKEN_TO))) {
		print_msg("failed to add profile\n");
		return NULL;
	}

	if ((rc = snx_json_obj_add_str(root, "auid", AUID))) {
		print_msg("failed to add profile\n");
		return NULL;
	}

	*output = snx_json_object_to_json_string(root);

	return root;
}

static int test_decomp_test_smaple(snx_json_t *root_obj)
{
	int rc = 0;
	int i = 0;
	char *out_string;
	int heartbeat;
	snx_json_t *jsobj_arry_test_int, *jsobj_arry_test_string;
	int num_test_int, num_test_string;
	char **list_string;

	if (!root_obj)
	{
		return -1;
	}

	if ((rc = snx_json_obj_get_int(root_obj, "Heartbeat", &heartbeat)))
	{
		print_msg("Couldn't get JSON int obj \n");
		snx_json_obj_free(root_obj);
	}
	print_msg("Heartbeat = %d\n", heartbeat);

	if ((rc = snx_json_obj_get_str(root_obj, "Message Server", &out_string)))
	{
		print_msg("Couldn't get JSON string obj \n");
		snx_json_obj_free(root_obj);
	}
	print_msg("Message Server = %s\n", out_string);

	if ((rc = snx_json_obj_get_array(root_obj, "Test Array integer", &jsobj_arry_test_int, &num_test_int))) {
		print_msg("Couldn't get JSON array obj \n");
		snx_json_obj_free(root_obj);
	}

	print_msg("Test array int num = %d\n", num_test_int);

	for (i = 0; i < num_test_int; i++)
	{
		int value;
		snx_json_obj_get_array_int(jsobj_arry_test_int, i, &value);
		print_msg("Test array int %d\n", value);
	}

	if ((rc = snx_json_obj_get_array(root_obj, "Test array string", &jsobj_arry_test_string, &num_test_string))) {
		print_msg("Couldn't get JSON array obj \n");
		snx_json_obj_free(root_obj);
	}

	print_msg("Test array string num = %d\n", num_test_string);

	list_string = malloc(sizeof(char *)*num_test_string);

	for (i = 0; i < num_test_string; i++)
	{
		snx_json_obj_get_array_str(jsobj_arry_test_string, i, &list_string[i]);
		print_msg("Test array string %s\n", list_string[i]);
	}

	if (list_string)
	{
		free(list_string);
	}
	return 0;
}

static int test_decomp_test_smaple2(snx_json_t *root_obj)
{
	int rc = 0;
	int version;
	char *namespace, *control, *session, *auth, *msgname, *msguid, *client, *profile, *app_type, *token, *tokento, *auid;
	snx_json_t *jheader, *jreply, *jmessage, *jaddress, *jtrack;
	int i = 0;
	int num_address = 0, num_track = 0;
	char **address_lsit;
	if (!root_obj)
	{
		return -1;
	}

	if ((rc = snx_json_obj_get_str(root_obj, "namespace", &namespace)))
	{
		print_msg("Couldn't get JSON string obj \n");
		snx_json_obj_free(root_obj);
	}
	print_msg("namespace = %s\n", namespace);

	/* ---------------------- header part ---------------------- */
	if ((rc = snx_json_obj_get_obj(root_obj, "header", &jheader)) != 0)
	{
		print_msg("Couldn't get header obj \n");
		return -1;
	}

	if ((rc = snx_json_obj_get_int(jheader, "version", &version))) {
		print_msg("failed to add version\n");
		return -1;
	}
	print_msg("version = %d\n", version);

	if ((rc = snx_json_obj_get_str(jheader, "name", &control))) {
		print_msg("failed to add name\n");
		return -1;
	}
	print_msg("name = %s\n", control);

	if ((rc = snx_json_obj_get_str(jheader, "session", &session))) {
		print_msg("failed to add session\n");
		return -1;
	}
	print_msg("session = %s\n", session);

	if ((rc = snx_json_obj_get_str(jheader, "auth", &auth))) {
		print_msg("failed to add session\n");
		return -1;
	}
	print_msg("auth = %s\n", auth);

	if ((rc = snx_json_obj_get_obj(jheader, "reply", &jreply)) != 0)
	{
		print_msg("Couldn't get reply obj \n");
		return -1;
	}

	if ((rc = snx_json_obj_get_str(jreply, "server", &msgname))) {
		print_msg("failed to add session\n");
		return -1;
	}
	print_msg("server = %s\n", msgname);

	if ((rc = snx_json_obj_get_str(jreply, "uid", &msguid))) {
		print_msg("failed to add session\n");
		return -1;
	}
	print_msg("uid = %s\n", msguid);

	/* ---------------------- message part ---------------------- */

	if ((rc = snx_json_obj_get_obj(root_obj, "message", &jmessage)) != 0)
	{
		print_msg("Couldn't get message obj \n");
		return -1;
	}

	if ((rc = snx_json_obj_get_str(jmessage, "client", &client))) {
		print_msg("failed to add client\n");
		return -1;
	}
	print_msg("client = %s\n", client);

	if ((rc = snx_json_obj_get_str(jmessage, "profile", &profile))) {
		print_msg("failed to add profile\n");
		return -1;
	}
	print_msg("profile = %s\n", profile);

	if ((rc = snx_json_obj_get_array(jmessage, "addresses", &jaddress, &num_address)) != 0)
	{
		print_msg("Couldn't add JSON address obj \n");
		return -1;
	}

	print_msg("num_address = %d\n", num_address);

	address_lsit = malloc(sizeof(char *)*num_address);

	for (i = 0; i < num_address; i++)
	{
		snx_json_obj_get_array_str(jaddress, i, &address_lsit[i]);
		print_msg("address %s\n", address_lsit[i]);
	}


	if ((rc = snx_json_obj_get_array(jmessage, "tracks", &jtrack, &num_track)) != 0)
	{
		print_msg("Couldn't get JSON track obj \n");
		return -1;
	}
	print_msg("num_track = %d\n", num_track);

	for (i = 0; i < num_track; i++)
	{
		int j = 0;
		int timeout = 0;
		int num_port = 0, num_pport = 0;
		char *track_name;
		snx_json_t *jtrack_c, *jport, *jpport;
		if ((rc = snx_json_obj_get_array_obj(jtrack, i, &jtrack_c)) != 0)
		{
			print_msg("Couldn't get JSON track content obj \n");
			return -1;
		}

		if ((rc = snx_json_obj_get_str(jtrack_c, "name", &track_name))) {
			print_msg("failed to add profile\n");
			return -1;
		}
		print_msg("track name = %s\n", track_name);

		if ((rc = snx_json_obj_get_array(jtrack_c, "port", &jport, &num_port)) != 0)
		{
			print_msg("Couldn't add JSON port obj \n");
			return -1;
		}

		print_msg("num_port = %d\n", num_port);
		for (j = 0; j < num_port; j++)
		{
			int value;
			snx_json_obj_get_array_int(jport, j, &value);
			print_msg("port =  %d\n", value);
		}

		if ((rc = snx_json_obj_get_array(jtrack_c, "public port", &jpport, &num_pport)) != 0)
		{
			print_msg("Couldn't add JSON port obj \n");
			return -1;
		}

		print_msg("num_pport = %d\n", num_pport);
		for (j = 0; j < num_pport; j++)
		{
			int value;
			snx_json_obj_get_array_int(jpport, j, &value);
			print_msg("public port =  %d\n", value);
		}

		if ((rc = snx_json_obj_get_int(jtrack_c, "timeout", &timeout))) {
			print_msg("failed to add profile\n");
			return -1;
		}
		print_msg("timeout =  %d\n", timeout);
	}


	if ((rc = snx_json_obj_get_str(root_obj, "type", &app_type))) {
		print_msg("failed to add profile\n");
		return -1;
	}
	print_msg("type = %s\n", app_type);

	if ((rc = snx_json_obj_get_str(root_obj, "toekn", &token))) {
		print_msg("failed to add profile\n");
		return -1;
	}
	print_msg("toekn = %s\n", token);

	if ((rc = snx_json_obj_get_str(root_obj, "toeknto", &tokento))) {
		print_msg("failed to add profile\n");
		return -1;
	}
	print_msg("toeknto = %s\n", tokento);

	if ((rc = snx_json_obj_get_str(root_obj, "auid", &auid))) {
		print_msg("failed to add profile\n");
		return -1;
	}
	print_msg("auid = %s\n", auid);

	return 0;
}

