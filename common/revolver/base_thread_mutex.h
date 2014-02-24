#ifndef __BASE_THREAD_MUTEX_H
#define __BASE_THREAD_MUTEX_H

#include "base_namespace.h"

#if defined(WIN32) | defined(_WIN32) | defined(_WIN64)

#if _WIN32_WINNT < 0x0502
#define _WIN32_WINNT 0x0502
#endif

#include <winsock2.h>
#include <windows.h>
#include <winbase.h>

BASE_NAMESPACE_BEGIN_DECL

class BaseThreadMutex
{
public:
	BaseThreadMutex()
	{
		InitializeCriticalSection(&section_);
	};

	~BaseThreadMutex()
	{
		DeleteCriticalSection(&section_);
	};

	void acquire()
	{
		EnterCriticalSection(&section_);
	};

	bool try_acquire()
	{
		return (TryEnterCriticalSection(&section_) != FALSE) ? true : false;
	};

	void release()
	{
		LeaveCriticalSection(&section_);
	};

private:
	CRITICAL_SECTION section_;
};

BASE_NAMESPACE_END_DECL;
#else
#include <pthread.h>

BASE_NAMESPACE_BEGIN_DECL
class BaseThreadMutex
{
public:
	BaseThreadMutex()
	{
		pthread_mutexattr_t mutex_a;
		pthread_mutexattr_init(&mutex_a);
		pthread_mutexattr_settype(&mutex_a, PTHREAD_MUTEX_RECURSIVE);

		pthread_mutex_init(&mutex_, &mutex_a);
		pthread_mutexattr_destroy(&mutex_a);
	};

	~BaseThreadMutex()
	{
		pthread_mutex_destroy(&mutex_);
	};
	
	void acquire()
	{
		pthread_mutex_lock(&mutex_);
	}

	bool try_acquire()
	{
		return pthread_mutex_trylock(&mutex_) == 0 ? true : false;
	}

	void release()
	{
		pthread_mutex_unlock(&mutex_);
	}

private:
	pthread_mutex_t mutex_;
};
BASE_NAMESPACE_END_DECL;
#endif

#endif
