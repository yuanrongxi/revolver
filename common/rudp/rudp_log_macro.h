#ifndef __RUDP_LOG_MACRO_H_
#define __RUDP_LOG_MACRO_H_

#include "revolver/base_log.h"
#include "revolver/base_hex_string.h"

extern SingleRotatedLogStream rudp_log;
extern SingleRotatedLogStream rudp_recv_log;
extern SingleRotatedLogStream rudp_send_log;
extern SingleRotatedLogStream rudp_trace_log;

#define RUDP_TRACE(arg)\
	TRACE_TRACE(rudp_trace_log, arg)

#define SET_RUDP_TRACE_LEVEL(arg)\
	rudp_trace_log.set_trace_level(arg)

#define RUDP_DEBUG(arg)\
	DEBUG_TRACE(rudp_log, arg)

#define RUDP_INFO(arg)\
	INFO_TRACE(rudp_log, arg)

#define RUDP_WARNING(arg)\
	WARNING_TRACE(rudp_log, arg)

#define RUDP_ERROR(arg)\
	ERROR_TRACE(rudp_log, arg)

#define RUDP_FATAL(arg)\
	FATAL_TRACE(rudp_log, arg)

#define SET_RUDP_LEVEL(arg)\
	rudp_log.set_trace_level(arg)


#define RUDP_RECV_DEBUG(arg)\
	DEBUG_TRACE(rudp_recv_log, arg)

#define RUDP_RECV_INFO(arg)\
	INFO_TRACE(rudp_recv_log, arg)

#define RUDP_RECV_WARNING(arg)\
	WARNING_TRACE(rudp_recv_log, arg)

#define RUDP_RECV_ERROR(arg)\
	ERROR_TRACE(rudp_recv_log, arg)

#define RUDP_RECV_FATAL(arg)\
	FATAL_TRACE(rudp_recv_log, arg)

#define SET_RUDP_RECV_LEVEL(arg)\
	rudp_recv_log.set_trace_level(arg)


#define RUDP_SEND_DEBUG(arg)\
	DEBUG_TRACE(rudp_send_log, arg)

#define RUDP_SEND_INFO(arg)\
	INFO_TRACE(rudp_send_log, arg)

#define RUDP_SEND_WARNING(arg)\
	WARNING_TRACE(rudp_send_log, arg)

#define RUDP_SEND_ERROR(arg)\
	ERROR_TRACE(rudp_send_log, arg)

#define RUDP_SEND_FATAL(arg)\
	FATAL_TRACE(rudp_send_log, arg)

#define SET_RUDP_SEND_LEVEL(arg)\
	rudp_send_log.set_trace_level(arg)

void change_rudp_log_path(const char* file_path);

#endif
