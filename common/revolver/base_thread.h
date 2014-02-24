/*************************************************************************************
*filename:	base_thread.h
*
*to do:		定义线程基类
*Create on: 2012-05
*Author:	来源于VDC LIBFRAME库
*check list:
*************************************************************************************/
#ifndef __BASE_THREAD_H
#define __BASE_THREAD_H

#include "base_namespace.h"
#include "base_os.h"

//包含PTHREAD库
#ifndef WIN32
#include <pthread.h>
#endif

#include <string>
#include <iostream>
using namespace std;

BASE_NAMESPACE_BEGIN_DECL

class CThread
{
public:
	CThread();
	virtual ~CThread();

public:
	int32_t			start();
	virtual int32_t	terminate();
	virtual void	execute();

	bool			get_terminated() const;
	void			set_terminated(bool terminated);

	bool			set_priority(int32_t pri);
	int32_t			get_priority() const;

	void			set_thread_index(int32_t thr_index);
	int32_t			get_thread_index() const;

	void			set_thread_name(const string& thr_name);
	const string&	get_thread_name() const;

	friend std::ostream& operator<<(std::ostream& os, const CThread& thr);

protected:
	void			clear_thread();

protected:
#ifdef WIN32
	HANDLE			thr_handle_;
	uint32_t		time_delay_;
#else
	pthread_t       thr_handle_;
	pthread_mutex_t thr_mutex_;
#endif
	bool			terminated_;
	int32_t			thr_index_;
	string			thr_name_;
};


BASE_NAMESPACE_END_DECL

#endif
/************************************************************************************/

