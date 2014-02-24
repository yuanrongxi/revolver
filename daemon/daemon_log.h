#ifndef __DAEMON_LOG_H
#define __DAEMON_LOG_H

#include "base_log.h"

extern BaseLogStream daemon_log;

#define DAEMON_DEBUG(arg)\
	DEBUG_TRACE(daemon_log, arg)

#define DAEMON_INFO(arg)\
	INFO_TRACE(daemon_log, arg)

#define DAEMON_WARNING(arg)\
	WARNING_TRACE(daemon_log, arg)

#define DAEMON_ERROR(arg)\
	ERROR_TRACE(daemon_log, arg)

#define DAEMON_FATAL(arg)\
	FATAL_TRACE(daemon_log, arg)

#define SET_DAEMON_LOG_LEVEL(arg)\
	daemon_log.set_trace_level(arg)

#endif


