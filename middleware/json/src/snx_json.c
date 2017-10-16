/**
 * @file
 * A simple demo application.
 *
 * @author Betters Tsai
 */


#include <stddef.h>

#include <FreeRTOS.h>
#include <bsp.h>
#include <nonstdlib.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "snx_json.h"
#include "json_object.h"
#include "json_tokener.h"

#define jsn_err(x) print_msg("%s(%u): " x, __FUNCTION__, __LINE__)

#define jsn_err_2(x, y) print_msg("%s(%u) - %s: " x, __FUNCTION__, __LINE__, y)

snx_json_t *snx_json_obj_new(void)
{
	snx_json_t *jobj;

	return (jobj = (json_object_new_object()));
}

void snx_json_obj_free(snx_json_t *jobj)
{
	json_object_put(jobj);
};

int snx_json_obj_add_str(snx_json_t *jsrc, const char *key, const char *string)
{
	snx_json_t *tgt_obj;

	if (!jsrc || !key || !string)
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	tgt_obj =  json_object_new_string(string);
	if (!tgt_obj)
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	json_object_object_add(jsrc, key, tgt_obj);

	return 0;
}

int snx_json_obj_add_int(snx_json_t *jsrc, const char *key, int value)
{
	snx_json_t *tgt_obj;

	if (!jsrc || !key)
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	tgt_obj =  json_object_new_int(value);
	if (!tgt_obj)
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	json_object_object_add(jsrc, key, tgt_obj);

	return 0;
}

int snx_json_obj_add_obj(snx_json_t *jsrc, const char *key, snx_json_t **jnewobj)
{
	snx_json_t *new_obj;
	if (!jsrc || !key)
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	if (!(new_obj = json_object_new_object()))
	{
		jsn_err("Create new object error \n");
	}

	json_object_object_add(jsrc, key, new_obj);

	*jnewobj = new_obj;

	return 0;
}

int snx_json_obj_add_array(snx_json_t *jsrc, const char *key, snx_json_t **array)
{
	snx_json_t *arry_obj;

	if (!jsrc || !key)
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	if (!(arry_obj = json_object_new_array()))
	{
		jsn_err("Create array object error \n");
	}

	json_object_object_add(jsrc, key, arry_obj);

	*array = arry_obj;

	return 0;
}

int snx_json_obj_add_array_obj(snx_json_t *arry_src, snx_json_t **jnewobj)
{
	snx_json_t *new_obj;
	if (!arry_src)
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	if (!(new_obj = json_object_new_object()))
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	if (json_object_array_add(arry_src, new_obj) != 0)
	{
		json_object_put(new_obj);
		return -1;
	}

	*jnewobj = new_obj;

	return 0;
}

int snx_json_obj_add_array_str(snx_json_t *arry_src, const char *string)
{
	snx_json_t *arry_memb_obj;
	if (!arry_src || !string)
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	if (!(arry_memb_obj =  json_object_new_string(string)))
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	if (json_object_array_add(arry_src, arry_memb_obj) != 0)
	{
		json_object_put(arry_memb_obj);
		return -1;
	}
	return 0;
}

int snx_json_obj_add_array_int(snx_json_t *arry_src, int value)
{
	snx_json_t *arry_memb_obj;
	if (!arry_src)
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	if (!(arry_memb_obj =  json_object_new_int(value)))
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	if (json_object_array_add(arry_src, arry_memb_obj) != 0)
	{
		json_object_put(arry_memb_obj);
		return -1;
	}
	return 0;
}

int snx_json_obj_get_str(snx_json_t *jsrc, const char *key, char **out_string)
{
	snx_json_t *tgt_obj;

	if (!jsrc || !key)
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	if (!(tgt_obj = json_object_object_get(jsrc, key)))
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	*out_string = json_object_get_string(tgt_obj);
	return 0;
}

int snx_json_obj_get_int(snx_json_t *jsrc, const char *key, int *out_value)
{
	snx_json_t *tgt_obj;

	if (!jsrc || !key)
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	if (!(tgt_obj =json_object_object_get(jsrc, key)))
	{
		jsn_err_2("Invalid object\n", key);
		return -1;
	}

	*out_value = json_object_get_int(tgt_obj);

	return 0;
}

int snx_json_obj_get_obj(snx_json_t *jsrc, const char *key, snx_json_t **jout)
{
	snx_json_t *new_out;

	if (!jsrc || !key)
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	if (!(new_out =json_object_object_get(jsrc, key)))
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	*jout = new_out;

	return 0;
}

int snx_json_obj_get_array(snx_json_t *jsrc, const char *key, snx_json_t **arry_src, int *num_arry_memb)
{
	snx_json_t *arry_obj;

	if (!jsrc || !key)
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	if (!(arry_obj =json_object_object_get(jsrc, key)))
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	*num_arry_memb = json_object_array_length(arry_obj);

	*arry_src = arry_obj;

	return 0;
}

int snx_json_obj_get_array_obj(snx_json_t *arry_src, int arry_idx, snx_json_t **arry_memb)
{
	if (!arry_src)
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	*arry_memb = json_object_array_get_idx(arry_src, arry_idx);

	return 0;
}

int snx_json_obj_get_array_int(snx_json_t *arry_src, int arr_idx, int *out_value)
{
	if (!arry_src)
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	snx_json_t *arry_memb = json_object_array_get_idx(arry_src, arr_idx);

	*out_value = json_object_get_int(arry_memb);

	return 0;
}

int snx_json_obj_get_array_str(snx_json_t *arry_src, int arr_idx, char **out_string)
{
	if (!arry_src)
	{
		jsn_err("Invalid object\n");
		return -1;
	}

	snx_json_t *arry_memb = json_object_array_get_idx(arry_src, arr_idx);

	*out_string = json_object_get_string(arry_memb);

	return 0;

}


char* snx_json_object_to_json_string(snx_json_t *jobj)
{
	return(json_object_to_json_string(jobj));
}



void snx_json_tokener_reset(snx_jtoken_t *token)
{
	json_tokener_reset(token);
}


snx_jtoken_t *snx_json_token_new(void)
{
	return json_tokener_new();
}


snx_json_t *snx_json_token_parse(snx_jtoken_t *token, const char* string, int len_string)
{
	return (json_tokener_parse_ex(token, string, len_string));
}

void snx_json_tokener_free(snx_jtoken_t *token)
{
	json_tokener_free(token);
}
