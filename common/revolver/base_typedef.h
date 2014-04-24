#ifndef __BASE_TYPEDEF_H
#define __BASE_TYPEDEF_H

#ifdef __GNUC__
#include <stdint.h>
#include <inttypes.h>
#endif

#if defined(WIN32) && _MSC_VER >= 1700
#include <stdint.h>
#include <inttypes.h>
#endif

#if defined(WIN32) || defined(_WIN32)
#pragma warning(disable:4996)
#pragma warning(disable:4005)

#ifndef _STDINT
typedef char				int8_t;
#endif
#ifndef _STDINT
typedef short				int16_t;
#endif
#ifndef _STDINT
typedef int					int32_t;
#endif
#ifndef _STDINT
typedef long long			int64_t;
#endif

#ifndef _STDINT
typedef unsigned char		uint8_t;
#endif
#ifndef _STDINT
typedef unsigned short		uint16_t;
#endif
#ifndef _STDINT
typedef unsigned int		uint32_t;
#endif
#ifndef _STDINT
typedef unsigned long long	uint64_t;
#endif

#elif defined(_WIN64)
#ifndef _STDINT
typedef char				int8_t;
#endif
#ifndef _STDINT
typedef short				int16_t;
#endif
#ifndef _STDINT
typedef int					int32_t;
#endif
#ifndef _STDINT
typedef long				int64_t;
#endif

#ifndef _STDINT
typedef unsigned char		uint8_t;
#endif
#ifndef _STDINT
typedef unsigned short		uint16_t;
#endif
#ifndef _STDINT
typedef unsigned int		uint32_t;
#endif
#ifndef _STDINT
typedef unsigned long		uint64_t;
#endif
#endif

#ifndef core_max
#define core_max(a, b)		((a) > (b) ? (a) : (b))
#endif

#ifndef core_min
#define core_min(a, b)		((a) < (b) ? (a) : (b))
#endif

#ifndef core_abs
#define core_abs(a, b)		((a) > (b) ? (a - b) : (b - a))
#endif

#ifndef NULL
#define NULL	0
#endif

//定义池的初始化的大小
#ifdef WIN32

#ifndef CONNECTION_POOL_SIZE
#define CONNECTION_POOL_SIZE		32
#endif

#ifndef LOG_POOL_SIZE
#define LOG_POOL_SIZE				32
#endif

#ifndef TIMER_POOL_SIZE
#define TIMER_POOL_SIZE				32
#endif

#ifndef STREAM_POOL_SIZE
#define STREAM_POOL_SIZE			16
#endif

#ifndef HANDLER_POOL_SIZE
#define HANDLER_POOL_SIZE			32
#endif

#ifndef RUDP_SOCKET_POOL_SIZE
#define RUDP_SOCKET_POOL_SIZE		32
#endif

#ifndef RUDP_SEGMENT_POOL_SIZE
#define RUDP_SEGMENT_POOL_SIZE		16
#endif

#ifndef LZO_VMEM_SISE
#define LZO_VMEM_SISE			(1024 * 1024)
#endif

#else

#ifndef CONNECTION_POOL_SIZE
#define CONNECTION_POOL_SIZE		1024
#endif

#ifndef LOG_POOL_SIZE
#define LOG_POOL_SIZE				96
#endif

#ifndef TIMER_POOL_SIZE
#define TIMER_POOL_SIZE				2048
#endif

#ifndef STREAM_POOL_SIZE
#define STREAM_POOL_SIZE			96
#endif

#ifndef HANDLER_POOL_SIZE
#define HANDLER_POOL_SIZE			1024
#endif

#ifndef RUDP_SOCKET_POOL_SIZE
#define RUDP_SOCKET_POOL_SIZE		1024
#endif 

#ifndef RUDP_SEGMENT_POOL_SIZE
#define RUDP_SEGMENT_POOL_SIZE		1024
#endif

#ifndef LZO_VMEM_SISE
#define LZO_VMEM_SISE			(1024 * 1024 * 4)
#endif

#endif

#endif
