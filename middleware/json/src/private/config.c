#include <nonstdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "config.h"

void *snx_calloc(size_t nmemb, size_t size)
{
	void *calloc_mem = malloc(nmemb*size);
	if (!calloc_mem)
	{
		return NULL;
	}
	memset(calloc_mem, 0x0, nmemb*size);
	return calloc_mem;
}

void *snx_realloc(void* ptr, size_t new_size)
{
	void *new_mem = malloc(new_size);
	if (!new_mem)
	{
		return NULL;
	}

	memcpy(new_mem, ptr, new_size);

	free(ptr);
	return new_mem;
}

#define MINUS	10
#define EXP		11

static int check_legal_character_range(const char *cptr)
{
	if (!cptr)
	{
		return -1;
	}

	if (isdigit(*cptr))
	{
		return (isdigit(*cptr));
	}
	else if (*cptr == '-')
	{
		return MINUS;
	}
	else if (*cptr == 'E' || *cptr == 'e')
	{
		return EXP;
	}
	else
		return -1;

}

static double pow(double x, double y)
{
	int i;
	double value = 0;
	int count = 0;
	if (x < 0)
		return -1;

	if (x == 0)
		return 0;

	if(y == 0)
		return 1;

	count = (y > 0)?(y):(-y);

	value = x;

	for ( i = 1; i < y; i++)
		value = value*x;

	value = (y > 0)?(value):(1/value);

	print_msg_queue("value = %f\n", value);

	return value;
}

double snx_strtod(const char *nptr, char **endptr)
{
	double value = 0;
	double exp_val =0 ;
	int rc = 0;
	int i, len = strlen(nptr);
	int sign = 0;
	int exp_on = 0;
	char *ptr;

	ptr = nptr;

	while(len > 0)
	{
		if ((rc = check_legal_character_range(ptr)) < 0)
			break;

		if (rc == MINUS)
			sign = 1;
		else if (rc == EXP)
			exp_on = 1;
		else
		{
			if (exp_on)
				exp_val = exp_val*10 + rc;
			else
				value = value*10 + rc;
		}

		len --;
	}

	value = (sign == 1)?(-value):(value);
	value = (exp_on == 1)?(value*pow(10, exp_val)):(value);

	print_msg_queue("value = %f\n", value);

	return value;
}

int snx_isspace(int c)
{
	//print_msg_queue("%d\n", c);
	if (c == 32 || c == 0 || c == 9 || c == 10 || c == 11 || c == 12)
	{
		return 1;
	}

	return 0;
}

int json_parse_int64(const char *buf, int64_t *retval)
{
	//int64_t num64;
	int32_t num64;
	const char *buf_sig_digits;
	int orig_has_neg;
	int saved_errno = 0;

	// Skip leading spaces
	while (snx_isspace((int)*buf) && *buf)
		buf++;
/*
	if (sscanf(buf, "%" SCNd64, &num64) != 1)
	{
		//MC_DEBUG("Failed to parse, sscanf != 1\n");
		return 1;
	}
*/
	sscanf(buf, "%"SCNd64, &num64);
	//print_msg_queue("++num64 = %d\n", num64);
	//saved_errno = errno;
	buf_sig_digits = buf;
	orig_has_neg = 0;
	if (*buf_sig_digits == '-')
	{
		buf_sig_digits++;
		orig_has_neg = 1;
	}

	// Not all sscanf implementations actually work
	//if (sscanf_is_broken && saved_errno != ERANGE)
	{
		char buf_cmp[100];
		char *buf_cmp_start = buf_cmp;
		int recheck_has_neg = 0;
		int buf_cmp_len;

		// Skip leading zeros, but keep at least one digit
		while (buf_sig_digits[0] == '0' && buf_sig_digits[1] != '\0')
			buf_sig_digits++;
		if (num64 == 0) // assume all sscanf impl's will parse -0 to 0
			orig_has_neg = 0; // "-0" is the same as just plain "0"

		snprintf(buf_cmp_start, sizeof(buf_cmp), "%"PRId64, num64);
		//print_msg_queue("--num64 = %d\n", num64);
		if (*buf_cmp_start == '-')
		{
			recheck_has_neg = 1;
			buf_cmp_start++;
		}
		// No need to skip leading spaces or zeros here.

		buf_cmp_len = strlen(buf_cmp_start);
		/**
		 * If the sign is different, or
		 * some of the digits are different, or
		 * there is another digit present in the original string
		 * then we have NOT successfully parsed the value.
		 */
		if (orig_has_neg != recheck_has_neg ||
		    strncmp(buf_sig_digits, buf_cmp_start, strlen(buf_cmp_start)) != 0 ||
			((int)strlen(buf_sig_digits) != buf_cmp_len &&
			 isdigit((int)buf_sig_digits[buf_cmp_len])
		    )
		   )
		{
			saved_errno = -1;
		}
	}

	// Not all sscanf impl's set the value properly when out of range.
	// Always do this, even for properly functioning implementations,
	// since it shouldn't slow things down much.
	if (saved_errno == -1)
	{
		if (orig_has_neg)
			num64 = INT64_MIN;
		else
			num64 = INT64_MAX;
	}
	*retval = num64;
	return 0;
}

int json_parse_double(const char *buf, double *retval)
{
  return (sscanf(buf, "%lf", retval)==1 ? 0 : 1);
}
