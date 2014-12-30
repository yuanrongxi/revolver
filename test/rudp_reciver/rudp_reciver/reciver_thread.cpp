#include "revolver/base_reactor_instance.h"
#include "reciver_thread.h"

RecvThread::RecvThread()
{

}

RecvThread::~RecvThread()
{

}

void RecvThread::execute()
{
	while(!get_terminated())
	{
		REACTOR_INSTANCE()->event_loop();	
	}

	REACTOR_INSTANCE()->stop_event_loop();

	clear_thread();
}

