/*************************************************************************************
*filename:	base_select_reactor.h
*
*to do:		定义反应器接口
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __BASE_SELECT_REACTOR_H
#define __BASE_SELECT_REACTOR_H

#include "revolver/base_os.h"
#include "revolver/base_thread_mutex.h"
#include "revolver/base_reactor.h"
#include "revolver/base_singleton.h"

#include <map>
using namespace std;

BASE_NAMESPACE_BEGIN_DECL 

class CSelectReator : public CReactor
{
public:
	CSelectReator();
	virtual ~CSelectReator();

	int32_t		open_reactor(uint32_t number_of_handlers);
	int32_t		close_reactor();

	int32_t		event_loop();
	int32_t		stop_event_loop();
	
	int32_t		register_handler(CEventHandler *handler, uint32_t masks);
	int32_t		remove_handler(CEventHandler *handler, uint32_t masks);
	int32_t		delete_handler(CEventHandler *handler, bool del_event_obj = false);

	uint32_t	set_timer(CEventHandler *event_handler, const void *act, uint32_t delay);
	uint32_t	cancel_timer(uint32_t timer_id, const void **act);

protected:
	ReactorEventHandlerInfoPool		handler_pool_;

	ReactorEventHandlerMap			handler_map_;
	TIMEQUEUE						timer_queue_;	//定时器
	CTimerFunctor					functor_;

	uint32_t						max_handler_num_;

	uint32_t						select_delay_;

	//BaseThreadMutex					mutex_;

	//SOCKET集合
	fd_set							read_set;
	fd_set							write_set;
	fd_set							expeption_set;
};

BASE_NAMESPACE_END_DECL

#endif

/************************************************************************************/
