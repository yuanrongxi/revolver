/*************************************************************************************
*filename:	sample_log_macro.h
*
*to do:		定义一个日志,实现各个级别的日志记录
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __SAMPLE_LOG_MACRO_H
#define __SAMPLE_LOG_MACRO_H

#include "base_log.h"
#include "base_hex_string.h"

extern BaseLogStream sample_log;

#define SAM_DEBUG(arg)\
	DEBUG_TRACE(sample_log, arg)

#define SAM_INFO(arg)\
	INFO_TRACE(sample_log, arg)

#define SAM_WARNING(arg)\
	WARNING_TRACE(sample_log, arg)

#define SAM_ERROR(arg)\
	ERROR_TRACE(sample_log, arg)

#define SAM_FATAL(arg)\
	FATAL_TRACE(sample_log, arg)

#endif

/************************************************************************************/
