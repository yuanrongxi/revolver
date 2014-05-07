/*************************************************************************************
*filename:	core_reactor_thread.h
*
*to do:		反应器运行线程
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __CORE_REACTOR_THREAD_H
#define __CORE_REACTOR_THREAD_H

#include "revolver/base_namespace.h"
#include "revolver/base_typedef.h"
#include "revolver/base_thread.h"
#include "revolver/base_singleton.h"

BASE_NAMESPACE_BEGIN_DECL

class CCoreThread : public CThread
{
public:
	CCoreThread();
	virtual ~CCoreThread();
	
	void execute();
};

#define CREATE_REACTOR_THREAD	CSingleton<CCoreThread>::instance
#define REACTOR_THREAD			CSingleton<CCoreThread>::instance
#define DESTROY_REACTOR_THREAD	CSingleton<CCoreThread>::destroy


BASE_NAMESPACE_END_DECL
#endif
/************************************************************************************/
