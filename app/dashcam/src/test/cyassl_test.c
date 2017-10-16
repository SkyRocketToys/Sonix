#include <cyassl/ctaocrypt/aes.h>
#include <cyassl/ctaocrypt/des3.h>
#include <cyassl/ctaocrypt/rsa.h>
#include "rsa_key.h"

int aes_test(void)
{
    Aes enc;
    Aes dec;
    int  ret;

    const byte msg[] = { /* "Now is the time for all " w/o trailing 0 */
        0x6e,0x6f,0x77,0x20,0x69,0x73,0x20,0x74,
        0x68,0x65,0x20,0x74,0x69,0x6d,0x65,0x20,
        0x66,0x6f,0x72,0x20,0x61,0x6c,0x6c,0x20
    };

    const byte verify[] =
    {
        0x95,0x94,0x92,0x57,0x5f,0x42,0x81,0x53,
        0x2c,0xcc,0x9d,0x46,0x77,0xa2,0x33,0xcb
    };

    byte key[] = "0123456789abcdef   ";  /* align */
    byte iv[]  = "1234567890abcdef   ";  /* align */

    byte cipher[AES_BLOCK_SIZE * 4];
    byte plain [AES_BLOCK_SIZE * 4];

    ret = AesSetKey(&enc, key, AES_BLOCK_SIZE, iv, AES_ENCRYPTION);
    if (ret != 0)
        return -1001;
    ret = AesSetKey(&dec, key, AES_BLOCK_SIZE, iv, AES_DECRYPTION);
    if (ret != 0)
        return -1002;

    ret = AesCbcEncrypt(&enc, cipher, msg,   AES_BLOCK_SIZE);
    if (ret != 0)
        return -1005;
    ret = AesCbcDecrypt(&dec, plain, cipher, AES_BLOCK_SIZE);
    if (ret != 0)
        return -1006;

    if (memcmp(plain, msg, AES_BLOCK_SIZE))
        return -60;

    if (memcmp(cipher, verify, AES_BLOCK_SIZE))
        return -61;

    return 0;
}

int aes_test2(void)
{
    Aes enc;
    Aes dec;
    int  ret;
    const byte msg[] = { /* "Now is the time for all " w/o trailing 0 */
        0x6b,0xc1,0xbe,0xe2,0x2e,0x40,0x9f,0x96,
        0xe9,0x3d,0x7e,0x11,0x73,0x93,0x17,0x2a
    };

    const byte verify[] =
    {
       0x76,0x49,0xab,0xac,0x81,0x19,0xb2,0x46,
       0xce,0xe9,0x8e,0x9b,0x12,0xe9,0x19,0x7d
    };

    byte key[] =
    {
        0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,
        0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c
    };
    byte iv[]  =
    {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0B,0x0c,0x0d,0x0e,0x0f
    };

    byte cipher[AES_BLOCK_SIZE * 4];
    byte plain [AES_BLOCK_SIZE * 4];
    memset(cipher, 0, sizeof(cipher));
    memset(plain, 0, sizeof(plain));

    ret = AesSetKey(&enc, key, AES_BLOCK_SIZE, iv, AES_ENCRYPTION);
    if (ret != 0)
        return -1007;
    ret = AesSetKey(&dec, key, AES_BLOCK_SIZE, iv, AES_DECRYPTION);
    if (ret != 0)
        return -1008;

    ret = AesCbcEncrypt(&enc, cipher, msg,   AES_BLOCK_SIZE);
    if (ret != 0)
        return -1009;
    ret = AesCbcDecrypt(&dec, plain, cipher, AES_BLOCK_SIZE);
    if (ret != 0)
        return -1010;

    if (memcmp(plain, msg, AES_BLOCK_SIZE))
        return -62;

    if (memcmp(cipher, verify, AES_BLOCK_SIZE))
        return -63;

    return 0;
}

int des_test(void)
{
    const byte vector[] = { /* "now is the time for all " w/o trailing 0 */
        0x6e,0x6f,0x77,0x20,0x69,0x73,0x20,0x74,
        0x68,0x65,0x20,0x74,0x69,0x6d,0x65,0x20,
        0x66,0x6f,0x72,0x20,0x61,0x6c,0x6c,0x20
    };

    byte plain[24];
    byte cipher[24];

    Des enc;
    Des dec;

    const byte key[] =
    {
        0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef
    };

    const byte iv[] =
    {
        0x12,0x34,0x56,0x78,0x90,0xab,0xcd,0xef
    };

    const byte verify[] =
    {
        0x8b,0x7c,0x52,0xb0,0x01,0x2b,0x6c,0xb8,
        0x4f,0x0f,0xeb,0xf3,0xfb,0x5f,0x86,0x73,
        0x15,0x85,0xb3,0x22,0x4b,0x86,0x2b,0x4b
    };

    int ret;

    ret = Des_SetKey(&enc, key, iv, DES_ENCRYPTION);
    if (ret != 0)
        return -31;

    Des_CbcEncrypt(&enc, cipher, vector, sizeof(vector));
    ret = Des_SetKey(&dec, key, iv, DES_DECRYPTION);
    if (ret != 0)
        return -32;
    Des_CbcDecrypt(&dec, plain, cipher, sizeof(cipher));

    if (memcmp(plain, vector, sizeof(plain)))
        return -33;

    if (memcmp(cipher, verify, sizeof(cipher)))
        return -34;

    return 0;
}

int rsa_test(void){
	byte*   tmp;
    size_t bytes;
    RsaKey key;
    RNG    rng;
    word32 idx = 0;
    int    ret;
    byte   in[] = "Everyone gets Friday off.";
    word32 inLen = (word32)strlen((char*)in);
    byte   out[256];
    byte   plain[256];

	ret = InitRsaKey(&key, 0);
    if (ret != 0) {
        free(tmp);
        return -39;
    }
	bytes = sizeof(rsa_key_der);
    ret = RsaPrivateKeyDecode(rsa_key_der, &idx, &key, (word32)bytes);
    if (ret != 0) {
		print_msg("ret = %d\n", ret);
        free(tmp);
        return -41;
    }
    ret = InitRng(&rng);
    if (ret != 0) {
        free(tmp);
        return -42;
    }
    ret = RsaPublicEncrypt(in, inLen, out, sizeof(out), &key, &rng);
    if (ret < 0) {
        free(tmp);
        return -43;
    }
    ret = RsaPrivateDecrypt(out, ret, plain, sizeof(plain), &key);
    if (ret < 0) {
        free(tmp);
        return -44;
    }
    if (memcmp(plain, in, inLen)) {
        free(tmp);
        return -45;
    }
    return 0;

}


int cyassl_test(void){
	print_msg("\n\ncyassl_test start.....\n");
	int ret = 0;

    ret = aes_test();

    if(ret<0)
        print_msg("AES Test failed ret =%d\n",ret);
    else
        print_msg("AES Test success\n");


    ret = aes_test2();

    if(ret<0)
        print_msg("AES 2 Test failed ret =%d\n",ret);
    else
        print_msg("AES 2 Test success\n");
	
    ret = des_test();

    if(ret<0)
        print_msg("DES Test failed ret =%d\n",ret);
    else
        print_msg("Des Test success\n");

    ret = rsa_test();

    if(ret<0)
        print_msg("RSA Test failed ret =%d\n",ret);
    else
        print_msg("RSA Test success\n");

	print_msg("cyassl_test finished...\n\n");
	
	return 0;
}
