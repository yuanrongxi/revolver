#ifndef _AES_H
#define _AES_H


#ifdef __cplusplus
extern "C" {
#endif 


/*
128 位密钥支持长度为 16 个字符
192 位密钥支持长度为 24 个字符
256 位密钥支持长度为 32 个字符
*/

#define AESKEYLEN_128 16
#define AESKEYLEN_192 24
#define AESKEYLEN_256 32


#define AES_SIZE 16

typedef struct aes_context
{
    unsigned int erk[64];     /* encryption round keys */
    unsigned int drk[64];     /* decryption round keys */
    int nr;             /* number of rounds */
}
aes_context;

int  aes_set_key( aes_context *ctx, const unsigned char *key, int nbits );
void aes_encrypt( aes_context *ctx, const unsigned char input[AES_SIZE], unsigned char output[AES_SIZE] );
void aes_decrypt( aes_context *ctx, const unsigned char input[AES_SIZE], unsigned char output[AES_SIZE] );



#ifdef __cplusplus
}
#endif

#endif /* aes.h */

