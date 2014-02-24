#ifndef __RECIVER_THREAD_H_
#define __RECIVER_THREAD_H_

#include "base_namespace.h"
#include "base_typedef.h"
#include "base_thread.h"
#include "base_singleton.h"

using namespace BASE;

class RecvThread : public CThread
{
public:
	RecvThread();
	virtual ~RecvThread();

	void execute();
};

#define CREATE_RECV_THREAD	CSingleton<RecvThread>::instance
#define RECV_THREAD			CSingleton<RecvThread>::instance
#define DESTROY_RECV_THREAD	CSingleton<RecvThread>::destroy

#endif


