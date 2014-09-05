#ifndef __RDUP_LOG_MACRO_H_
#define __RDUP_LOG_MACRO_H_

#include "revolver/base_log.h"
#include "revolver/base_hex_string.h"

extern SingleLogStream rdup_log;
extern SingleLogStream rdup_recv_log;
extern SingleLogStream rdup_send_log;

#define RUDP_DEBUG(arg)\
	DEBUG_TRACE(rdup_log, arg)

#define RUDP_INFO(arg)\
	INFO_TRACE(rdup_log, arg)

#define RUDP_WARNING(arg)\
	WARNING_TRACE(rdup_log, arg)

#define RUDP_ERROR(arg)\
	ERROR_TRACE(rdup_log, arg)

#define RUDP_FATAL(arg)\
	FATAL_TRACE(rdup_log, arg)

#define SET_RUDP_LEVEL(arg)\
	rdup_log.set_trace_level(arg)


#define RUDP_RECV_DEBUG(arg)\
	DEBUG_TRACE(rdup_recv_log, arg)

#define RUDP_RECV_INFO(arg)\
	INFO_TRACE(rdup_recv_log, arg)

#define RUDP_RECV_WARNING(arg)\
	WARNING_TRACE(rdup_recv_log, arg)

#define RUDP_RECV_ERROR(arg)\
	ERROR_TRACE(rdup_recv_log, arg)

#define RUDP_RECV_FATAL(arg)\
	FATAL_TRACE(rdup_recv_log, arg)

#define SET_RUDP_RECV_LEVEL(arg)\
	rdup_recv_log.set_trace_level(arg)


#define RUDP_SEND_DEBUG(arg)\
	DEBUG_TRACE(rdup_send_log, arg)

#define RUDP_SEND_INFO(arg)\
	INFO_TRACE(rdup_send_log, arg)

#define RUDP_SEND_WARNING(arg)\
	WARNING_TRACE(rdup_send_log, arg)

#define RUDP_SEND_ERROR(arg)\
	ERROR_TRACE(rdup_send_log, arg)

#define RUDP_SEND_FATAL(arg)\
	FATAL_TRACE(rdup_send_log, arg)

#define SET_RUDP_SEND_LEVEL(arg)\
	rdup_send_log.set_trace_level(arg)

#endif
