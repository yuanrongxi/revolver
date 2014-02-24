#ifdef __cplusplus
extern "C" {
#endif 

#include "crc_crypt.h"
#include <string.h>
#include <malloc.h>

#define CRC_FIRST		0x3b
#define CRC_SECOND		0xe6

static void head_crc(unsigned char* src, int src_size,
					 unsigned char* first, unsigned char* second)
{
	register int i = 0;
	*first = 0x3b;
	*second = 0xe6;

	for(; i < src_size; ++i)
	{
		if((i & 0x0001))
			*second = *second ^ src[i];
		else
			*first = *first ^ src[i];
	}
}

static void encode_xor(unsigned char* key, int key_size,
				unsigned char* src, int src_size,
				unsigned char* dst,
				unsigned char* first, unsigned char* second)
{
	register int i = 0;
	for(; i < src_size; ++i)
	{
		dst[i] = src[i] ^ key[i % key_size];
		if((i & 0x0001))
			*first = *first ^ src[i];
		else
			*second = *second ^ src[i];
	}
}

static void decode_xor(unsigned char* key, int key_size,
					   unsigned char* src, int src_size,
					   unsigned char* dst,
					   unsigned char* first, unsigned char* second)
{
	register int i = 0;
	for(; i < src_size; ++i)
	{
		dst[i] = src[i] ^ key[i % key_size];
		if((i & 0x0001))
			*second = *second ^ dst[i];
		else
			*first = *first ^ dst[i];
	}

}

aes_context* create_crypt(unsigned char* _ekey, int _ebits)
{
	aes_context* context = (aes_context*)malloc(sizeof(aes_context));
	aes_set_key(context, _ekey, _ebits);

	return context;
}

void destroy_crypt(aes_context* context)
{
	if(context != NULL)
	{
		free(context);
	}
}

int encrypt(aes_context* handler, unsigned char* src, int src_size, unsigned char* dst)
{
	unsigned char first = 0, second = 0;

	if(src_size < AES_SIZE)
	{
		//不进行加密，提示失败，最小报文必须是16字节
		return 0;
	}

	//计算CRC
	head_crc(src, AES_SIZE, &first, &second);
	if (src_size > AES_SIZE)
	{
		encode_xor(src, AES_SIZE, src + AES_SIZE, src_size - AES_SIZE, dst + AES_SIZE, &first, &second);
	}
	else
	{
		src_size = AES_SIZE;
	}

	//将头部16个字节进行aes加密.
	aes_encrypt(handler, src, dst);

	//添加CRC到报文当中
	*(dst + src_size) = first;
	*(dst + src_size + 1) = second;

	return src_size + 2;
}

int decrypt(aes_context* handler, unsigned char* src, int src_size, unsigned char* dst)
{
	unsigned char first = 0, second = 0;

	if(src_size < AES_SIZE + 2)
		return 0;

	//aes解密
	aes_decrypt(handler, src, dst);
	head_crc(dst, AES_SIZE, &first, &second);
	if(src_size > AES_SIZE + 2)
	{
		decode_xor(dst, AES_SIZE, src + AES_SIZE, src_size - AES_SIZE - 2, dst + AES_SIZE, &first, &second);
	}

	if(first == *(src + src_size - 2) && second == *(src + src_size - 1)) //校验和正确
	{
		return src_size - 2;
	}
	else
		return 0;
}

#ifdef __cplusplus
}
#endif
