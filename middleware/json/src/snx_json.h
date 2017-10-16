#ifndef __SNX_JSON_H__
#define __SNX_JSON_H__

/* pre */
typedef struct json_object snx_json_t;

typedef struct json_tokener snx_jtoken_t;

/* JSON tokener declartion */
void snx_json_tokener_reset(snx_jtoken_t *token);
snx_jtoken_t *snx_json_token_new(void);
snx_json_t *snx_json_token_parse(snx_jtoken_t *token, const char* string, int len_string);
void snx_json_tokener_free(snx_jtoken_t *token);

/* JSON object declartion */
snx_json_t *snx_json_obj_new(void);
void snx_json_obj_free(snx_json_t *jobj);

int snx_json_obj_add_str(snx_json_t *jsrc, const char *key, const char *string);
int snx_json_obj_add_int(snx_json_t *jsrc, const char *key, int value);
int snx_json_obj_add_obj(snx_json_t *jsrc, const char *key, snx_json_t **jnewobj);
int snx_json_obj_add_array(snx_json_t *jsrc, const char *key, snx_json_t **array);
int snx_json_obj_add_array_obj(snx_json_t *arry_src, snx_json_t **jnewobj);
int snx_json_obj_add_array_str(snx_json_t *arry_src, const char *string);
int snx_json_obj_add_array_int(snx_json_t *arry_src, int value);


int snx_json_obj_get_str(snx_json_t *jsrc, const char *key, char **out_string);
int snx_json_obj_get_int(snx_json_t *jsrc, const char *key, int *out_value);
int snx_json_obj_get_obj(snx_json_t *jsrc, const char *key, snx_json_t **jout);
int snx_json_obj_get_array(snx_json_t *jsrc, const char *key, snx_json_t **arry_src, int *num_arry_memb);
int snx_json_obj_get_array_obj(snx_json_t *arry_src, int arry_idx, snx_json_t **arry_memb);
int snx_json_obj_get_array_int(snx_json_t *arry_src, int arr_idx, int *out_value);
int snx_json_obj_get_array_str(snx_json_t *arry_src, int arr_idx, char **out_value);


char* snx_json_object_to_json_string(snx_json_t *jobj);
#endif
