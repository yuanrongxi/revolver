#include "base_os.h"
#include "base_thread.h"
#include <time.h>

BASE_NAMESPACE_BEGIN_DECL

#define THREAD_TERMINATE_TIMEOUT	2000

#ifdef WIN32
void thread_proc(void *param)
{
	if(param != NULL)
	{
		::CoInitialize(NULL);

		srand((uint32_t)time(NULL));

		CThread* thr = (CThread *)param;
		thr->execute();

		::CoUninitialize();
	}
}
#else
void* thread_proc(void *param)
{
	if(param != NULL)
	{
		srand((uint32_t)time(NULL));
		srandom((uint32_t)time(NULL));

		CThread* thr = (CThread *)param;
		thr->execute();
	}

	return NULL;
}
#endif

CThread::CThread() : terminated_(false), thr_name_(""), thr_index_(-1)
{
#ifdef WIN32
	time_delay_ = THREAD_TERMINATE_TIMEOUT;
	thr_handle_ = NULL;
#else
	thr_handle_ = 0;
#endif
}

CThread::~CThread()
{
}

int32_t CThread::start()
{
	terminated_ = false;

#ifdef WIN32
	thr_handle_ = (HANDLE)_beginthread(thread_proc, 0,this);
#else
	pthread_create(&thr_handle_, NULL, thread_proc, this);
#endif

	return 0;
}

int32_t CThread::terminate()
{
	if(terminated_)
	{
		return 0;
	}

	terminated_ = true;

#ifdef WIN32
	if(thr_handle_ != NULL)
	{
		WaitForSingleObject(thr_handle_, time_delay_);
	}
#else
	if(thr_handle_ != 0)
	{
		pthread_join(thr_handle_, NULL);
	}
#endif

	return 0;
}

void CThread::execute()
{

}

void CThread::clear_thread()
{
#ifdef WIN32
	_endthread();
#else
	pthread_exit(NULL);
#endif
}

int32_t CThread::get_priority() const
{
	return 0;
}

bool CThread::set_priority(int32_t pri)
{
	return true;
}

bool CThread::get_terminated() const
{
	return terminated_;
}

void CThread::set_terminated(bool terminated)
{
	terminated_ = terminated;
}

void CThread::set_thread_index(int32_t thr_index)
{
	thr_index_ = thr_index;
}

int32_t CThread::get_thread_index() const
{
	return thr_index_;
}

void CThread::set_thread_name(const string& thr_name)
{
	thr_name_= thr_name;
}

const string& CThread::get_thread_name() const
{
	return thr_name_;
}

std::ostream&	operator<< (std::ostream& os, const CThread& thr)
{
//	os << "thread info, handler = " << (uint32_t)thr.thr_handle_ \
//		<< ", terminated = " << (thr.terminated_ ? "true" : "false") \
//		<< ", thr_index = " << thr.thr_index_ \
//		<< ", thr_name = " << thr.thr_name_ << "\n";

	return os;
}

BASE_NAMESPACE_END_DECL
