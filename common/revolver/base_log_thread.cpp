#include "base_log_thread.h"
#include "base_log.h"
#include <list>

ObjectMutexPool<LogInfoData, BaseThreadMutex, LOG_POOL_SIZE>	LOGPOOL;

BaseLogThread::BaseLogThread()
{

}

BaseLogThread::~BaseLogThread()
{
	clear();
}

void BaseLogThread::clear()
{
	LogInfoData* data = NULL;
	while(queue_.get(data))
	{
		data->reset();
		LOGPOOL.push_obj(data);
	}
}

void BaseLogThread::put_log(LogInfoData* data)
{
	if(!queue_.put(data))
	{
		data->reset();
		LOGPOOL.push_obj(data);
	}
}

//用于非正常退出！！！
void BaseLogThread::stop()
{
	terminated_ = true;
}


void BaseLogThread::execute()
{
	LogInfoData* data = NULL;

	while(!get_terminated())
	{
		data = NULL;

		if(queue_.get(data))
		{
			if(data == NULL)
				continue;

			BaseLog* log = LOG_INSTANCE()->get_log_handler(data->index);
			if(log != NULL)
				log->write_log(data->str_log);

			data->reset();
			LOGPOOL.push_obj(data);
		}
		else
		{
			usleep(1000);
		}

		LOG_INSTANCE()->flush();
	}
}

