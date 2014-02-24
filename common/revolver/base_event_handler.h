/*************************************************************************************
*filename:	base_event_handler.h
*
*to do:		定义事件器接口
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __BASE_EVENT_HANDLER_H
#define __BASE_EVENT_HANDLER_H

#include "base_namespace.h"
#include "base_typedef.h"
#include "base_os.h"

#include <set>

using namespace std;

BASE_NAMESPACE_BEGIN_DECL

typedef enum EVENT_MASK
{
	MASK_READ		= (1 << 0), //读事件
	MASK_WRITE		= (1 << 1),	//写事件
	MASK_TIMEOUT	= (1 << 2),	//超时事件
	MASK_EXCEPT		= (1 << 3),	//异常事件
}EVENT_MASK;

typedef uint32_t ReactorMask;

class CReactor;

typedef set<uint32_t>		BaseEventIDSet;

class CEventHandler
{
public:
	CEventHandler();
	virtual ~CEventHandler();
	
public:
	virtual BASE_HANDLER	get_handle() const;
	virtual void			set_handle(BASE_HANDLER handle);
	
	uint32_t				get_reactor_index() const {return reactor_index_;};	
	void					set_reactor_index(uint32_t index) {reactor_index_ = index;};

	void					add_timer_event(uint32_t id);
	void					del_timer_event(uint32_t id);

public:
	virtual int32_t			handle_timeout(const void *act, uint32_t timer_id);
	virtual int32_t			handle_input(BASE_HANDLER handle);
	virtual int32_t			handle_output(BASE_HANDLER handle);
	virtual int32_t			handle_close(BASE_HANDLER handle, ReactorMask close_mask);
	virtual int32_t			handle_exception(BASE_HANDLER handle);

	void					reactor(CReactor *r);
	CReactor*				reactor (void) const;

protected:
	virtual void			release_timer_act(const void* act);
	void					clear_timer_events();
protected:
	CReactor*				reactor_;
	uint32_t				reactor_index_;			//在EPOLL HEAP上的索引，在SELECT下无用！！
	//TIMER ID
	BaseEventIDSet			timer_events_;
};

BASE_NAMESPACE_END_DECL
/************************************************************************************/
#endif
