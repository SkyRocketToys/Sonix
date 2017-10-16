#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "generated/snx_sdk_conf.h"

#define SHIFT_LEFT   0
#define SHIFT_RIGHT 1

typedef struct cyclic_shift {
	unsigned int ctr;	// SHIFT_LEFT / SHIFT_RIGHT
	unsigned int bit;
} cyclic_shift_t;

typedef struct exchange {
	unsigned int m;
	unsigned int n;
} exchange_t;

typedef struct xor {
	unsigned long long val[2];
} xor_t;

typedef enum op {
	BIT_CYCLIC_SHIFT,	// (shift left/right, n)
	BIT_EXCHANGE,		// (bit-m, bit-n)
	BIT_XOR,		// (XOR value)
	BIT_NOT
} op_t;

cyclic_shift_t var_shift;
exchange_t var_exchange;
xor_t var_xor;

int func (unsigned long long *key, op_t operation, void *args)
{
	switch (operation)
	{
	case BIT_CYCLIC_SHIFT:
		{
			unsigned long long of_val;
			unsigned int shift_bit = ((cyclic_shift_t *) args)->bit;

			if (shift_bit >= 64) {
				of_val = key[0];
				key[0] = key[1];
				key[1] = of_val;
				shift_bit -= 64;
			}

			if ( shift_bit==0 ) {
				break;
			}

			if (((cyclic_shift_t *) args)->ctr == SHIFT_LEFT) {
				of_val = key[1] >> ((8 * sizeof (key[1])) - shift_bit);
				key[1] = (key[1] << shift_bit) | (key[0] >> ((8 * sizeof (key[0])) - shift_bit));
				key[0] = (key[0] << shift_bit) | of_val;
			} else /* SHIFT_RIGHT */{
				of_val = key[0] << ((8 * sizeof (key[1])) - shift_bit);
				key[0] = (key[0] >> shift_bit) | (key[1] << ((8 * sizeof (key[1])) - shift_bit));
				key[1] = (key[1] >> shift_bit) | of_val;
			}
		}
		break;
	case BIT_EXCHANGE:
		{
			unsigned int bit1, bit2;
			unsigned int m_val, n_val;
			unsigned long long tmp;

			bit1 = ((exchange_t *) args)->m;
			bit2 = ((exchange_t *) args)->n;

			if (bit1 >= 64) {
				m_val = (key[1] & ((unsigned long long) 1 << (bit1 - 64))) >> (bit1 - 64);
			} else {
				tmp = (unsigned long long) 1 << bit1; 
				m_val = (key[0] & ((unsigned long long) 1 << bit1)) >> bit1;
				//printf ("m_val=%d, tmp=0x%llx, (1<<bit1)=0x%llx, key[0] = 0x%llx -> 0x%llx\n", m_val, tmp, (unsigned long long) 1<<bit1, key[0], key[0] & ( 1 << bit1));
			}

			if (bit2 >= 64) {
				n_val = (key[1] & ((unsigned long long) 1 << (bit2 - 64))) >> (bit2 - 64);
			} else {
				tmp = (unsigned long long) 1 << bit2; 
				n_val = (key[0] & ((unsigned long long) 1 << bit2)) >> bit2;
				//printf ("n_val=%d, tmp=0x%llx, (1<<n)=0x%llx, key[0] = 0x%llx -> 0x%llx\n", n_val, tmp, (unsigned long long) 1<<bit2, key[0], key[0] & ((unsigned long long) 1 << bit2));
			}
//			printf ("m=%d(%d), n=%d(%d)\n", m, m_val, n, n_val);
			if (m_val == n_val)		// no need to change
				break;

			if (bit1 >= 64) {
				key[1] &= ~((unsigned long long) 1 << (bit1 - 64));
				if (n_val)
					key[1] |= (unsigned long long) 1 << (bit1 - 64);
			} else {
				key[0] &= ~((unsigned long long) 1 << bit1);
				if (n_val)
					key[0] |= (unsigned long long) 1 << bit1;
			}

			if (bit2 >= 64) {
				key[1] &= ~((unsigned long long) 1 << (bit2 - 64));
				if (m_val)
					key[1] |= (unsigned long long) 1 << (bit2 - 64);
			} else {
				key[0] &= ~((unsigned long long) 1 << bit2);
				if (m_val)
					key[0] |= (unsigned long long) 1 << bit2;
			}
		}
		break;
	case BIT_XOR:
		key[0] ^= ((xor_t *) args)->val[0];
		key[1] ^= ((xor_t *) args)->val[1];
		break;
	case BIT_NOT:
		key[0] = ~key[0];
		key[1] = ~key[1];
		break;
	}

	return 0;
}
void snx_setkey()
{
	unsigned long long key=(unsigned long long)CONFIG_FW_ENCRYPT_KEY;
	
	// do cyclic shift right
	var_shift.bit =  (unsigned long long )key % (8 * 2 * sizeof (unsigned long long));

	// do bit exchange
	var_exchange.m =  (unsigned long long )key % (8 * 2 * sizeof (unsigned long long));
	var_exchange.n =  (unsigned long long )key % (8 * 2 * sizeof (unsigned long long));

	// do xor
	//var_xor.val[0] = (unsigned long long )key | ((unsigned long long )key << 32);	//0x123456789abcdef0;
	//var_xor.val[1] = (unsigned long long )key | ((unsigned long long )key << 32);	//0x23456789abcdef01;
	var_xor.val[0] = key;	
	var_xor.val[1] = key;
}

int snx_encrypt(unsigned long long *data)
{
	var_shift.ctr = SHIFT_RIGHT;
	func (data, BIT_CYCLIC_SHIFT, (void *)&var_shift);

	func (data, BIT_NOT, NULL);

//	func (data, BIT_EXCHANGE, &var_exchange);

	func (data, BIT_XOR, &var_xor);

	return 1;
}

int snx_decrypt(unsigned long long *data)
{
	func (data, BIT_XOR, &var_xor);

//	func (data, BIT_EXCHANGE, &var_exchange);

	func (data, BIT_NOT, NULL);
	
	var_shift.ctr = SHIFT_LEFT;
	func (data, BIT_CYCLIC_SHIFT, (void *)&var_shift);

	return 1;
}
int main(int argc,char *argv[])
{
	unsigned long long app_data[]={0x7890123456789012,0x1234567890123456};
	char *mch = NULL;
	FILE *fp;
	int i;

	char cmd_str[512];

	if (argc != 3)
		return -1;
//	data_size = 2* sizeof (unsigned long long);
	mch = argv[1];

	for (i=0;i<32;i++){
		printf("%c,", mch[i]);
	}

	printf("\n");

	memset(cmd_str, '\0', 512);
	sprintf(cmd_str, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", mch[0],mch[1],mch[2],mch[3],mch[4],mch[5],mch[6],mch[7],mch[8],mch[9],mch[10],mch[11],mch[12],mch[13],mch[14],mch[15]);
	sscanf(cmd_str, "%016llX", &app_data[1] );

  	memset(cmd_str, '\0', 512);
	sprintf(cmd_str, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", mch[16],mch[17],mch[18],mch[19],mch[20],mch[21],mch[22],mch[23],mch[24],mch[25],mch[26],mch[27],mch[28],mch[29],mch[30],mch[31]);
	sscanf(cmd_str, "%016llX", &app_data[0] );

	printf("app_data[0] = 0x%016llx\n", app_data[0]);  
	printf("app_data[1] = 0x%016llx\n", app_data[1]);

    //snx_decrypt
	snx_setkey();
//	printf("key=%08X\n\n",key);
	
	printf("Before encrypt\n");
	printf("app_data=0x%016llx%016llx\n\n",app_data[1],app_data[0]);

	snx_encrypt(app_data);
	printf("After encrypt\n");
	printf("app_data=0x%016llx%016llx\n\n",app_data[1],app_data[0]);


	fp = fopen(argv[2], "wb");
	fwrite(app_data, sizeof(unsigned long long), 2, fp);
	fclose (fp);

	return 0;
}

/*
[root@localhost /]# ./snx_encrypt_decrypt
Before encrypt
app_data=0x0f0e0d0c0b0a09080706050403020100

After encrypt
app_data=0xebedeff1f3f5f7f9fbfdffe1e3e5e7e9

After decrypt
app_data=0x0f0e0d0c0b0a09080706050403020100
*/
