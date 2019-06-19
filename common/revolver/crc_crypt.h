#ifndef __CRC_CRYPT_H
#define __CRC_CRYPT_H

#ifdef __cplusplus
extern "C" {
#endif 

#include "aes.h"

aes_context*	create_crypt(unsigned char* _ekey, int _ebits);
void			destroy_crypt(aes_context* context);

int				encrypt(aes_context* handler, unsigned char* src, int src_size, unsigned char* dst);
int				decrypt(aes_context* handler, unsigned char* src, int src_size, unsigned char* dst);

#ifdef __cplusplus
}
#endif

#endif
