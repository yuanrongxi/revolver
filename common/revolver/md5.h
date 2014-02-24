/*
 * md5.h        Structures and prototypes for md5.
 *
 * Version:     $Id: md5.h,v 1.3 2006/06/30 01:28:16 cvsroot Exp $
 * License:		LGPL, but largely derived from a public domain source.
 *
 */
#ifdef __cplusplus
extern "C" {
#endif


#ifndef _LRAD_MD5_H
#define _LRAD_MD5_H


void librad_md5_calc(unsigned char *output, unsigned char *input,unsigned int inputlen);

/*
 *  FreeRADIUS defines to ensure globally unique MD5 function names,
 *  so that we don't pick up vendor-specific broken MD5 libraries.
 */
#define MD5_CTX		librad_MD5_CTX
#define MD5Init		librad_MD5Init
#define MD5Update	librad_MD5Update
#define MD5Final	librad_MD5Final
#define MD5Transform	librad_MD5Transform
#define	MD5_BLOCK_LENGTH		64
#define	MD5_DIGEST_LENGTH		16

typedef unsigned int	uint32_t;
typedef unsigned char	uint8_t;

typedef struct MD5Context {
	uint32_t state[4];			/* state */
	uint32_t count[2];			/* number of bits, mod 2^64 */
	uint8_t buffer[MD5_BLOCK_LENGTH];	/* input buffer */
} MD5_CTX;

/* include <sys/cdefs.h> */

/* __BEGIN_DECLS */
void	 MD5Init(MD5_CTX *);
void	 MD5Update(MD5_CTX *, const uint8_t *, size_t)
/*		__attribute__((__bounded__(__string__,2,3)))*/;
void	 MD5Final(uint8_t [MD5_DIGEST_LENGTH], MD5_CTX *)
/*		__attribute__((__bounded__(__minbytes__,1,MD5_DIGEST_LENGTH)))*/;
void	 MD5Transform(uint32_t [4], const uint8_t [MD5_BLOCK_LENGTH])
/*		__attribute__((__bounded__(__minbytes__,1,4)))*/
/*		__attribute__((__bounded__(__minbytes__,2,MD5_BLOCK_LENGTH)))*/;
/* __END_DECLS */

#endif /* _LRAD_MD5_H */


#ifdef __cplusplus
}
#endif

