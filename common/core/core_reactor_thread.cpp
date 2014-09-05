#include "revolver/base_reactor_instance.h"
#include "core/core_reactor_thread.h"
BASE_NAMESPACE_BEGIN_DECL

CCoreThread::CCoreThread()
{

}

CCoreThread::~CCoreThread()
{

}

void CCoreThread::execute()
{
	while(!get_terminated())
	{
		REACTOR_INSTANCE()->event_loop();	
	}

	REACTOR_INSTANCE()->stop_event_loop();

	clear_thread();
}

BASE_NAMESPACE_END_DECL
