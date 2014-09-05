/*************************************************************************************
*filename:	timer_queue_t.h
*
*to do:		定义轮转定时器，实现一个4轮刻度转换的定时器轮序列，精确的最小单位为毫秒
*Create on: 2012-04
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __TIMER_QUEUE_T_H
#define __TIMER_QUEUE_T_H

#include "revolver/timer_ring.h"
#include "revolver/object_pool.h"
#include "revolver/timer_node_t.h"
#include "revolver/base_timer_value.h"
#include "revolver/base_event_handler.h"

#include <list>
#include <assert.h>

BASE_NAMESPACE_BEGIN_DECL
#define RINGS_SIZE	4

typedef std::list<uint32_t>	FreeTimerIDList;

template<class HANDLER, class FUNCTOR, class LOCK>
class CTimerQueue_T : public IRingEvent
{
public:
	typedef vector<BaseTimerNode_T<HANDLER>*>		TimerNodeArray;

	CTimerQueue_T(FUNCTOR* functor, size_t heap_size = TIMER_POOL_SIZE);
	~CTimerQueue_T();

	//插入一个定时器
	uint32_t schedule(HANDLER handler, const void* act, uint32_t delay, uint32_t interval = 0);
	//取消一个定时器
	void cancel_timer(uint32_t timer_id, const void **act);
	//重置一个定时器
	uint32_t reset_timer(uint32_t timer_id, uint32_t delay, uint32_t interval = 0);

	//扫描超时，返回下一个超时间间隔，最小为10MS，最大为50MS
	virtual uint32_t expire();

	//原子处理事务
	void ring_event(uint8_t ring_id, uint32_t timer_id);

	//判断定时器堆是否满
	bool full() const;

	FUNCTOR &upcall_functor (void);

#if _DEBUG
	//用于触发临界点，0,255,255,255
	void set_ring_id();
#endif

protected:
	void dispatch_info(BaseTimerDispathInfo_T<HANDLER>& info, uint32_t timer_id);
	//获取一个空闲的Timer node序号
	uint32_t get_free_node();

	void clear();

	void alloc_heap(size_t size);

	void insert_node(BaseTimerNode_T<HANDLER>* node);
	void delete_node(BaseTimerNode_T<HANDLER>* node);

	uint32_t revolver(uint32_t scale); 
protected:
	ObjectPool<BaseTimerNode_T<HANDLER>, TIMER_POOL_SIZE>	node_pool_;
	
	//定时器堆
	TimerNodeArray			heap_;
	size_t					heap_size_;
	//已经占用的HEAP个数
	uint32_t				used_num_;
	uint32_t				cur_heap_pos_;
	FreeTimerIDList			freeTimers_;

	//轮序列
	CTimerRing				rings_[RINGS_SIZE];	//0表示低位轮，3表示高位轮,一个轮表示256个刻度，4个轮正好是32位整形数描述

	//初始时刻
	CBaseTimeValue			start_time_;	
	//上一次expire处理的时刻
	CBaseTimeValue			prev_time_;

	LOCK					mutex_;
	FUNCTOR*				functor_;
};

//定义functor
class CTimerFunctor
{
public:

	CTimerFunctor()
	{

	}

	~CTimerFunctor()
	{

	}

	void registration(CEventHandler* handler, uint32_t id)
	{
		handler->add_timer_event(id);
	}

	void cancel_timer(CEventHandler *handler, uint32_t id)
	{
		handler->del_timer_event(id);
	}

	void timer_out(CEventHandler *event_handler,
		const void *act, 
		int32_t recurring_timer, 
		uint32_t timer_id)
	{
		event_handler->del_timer_event(timer_id);
		event_handler->handle_timeout(act, timer_id);
	}
};
BASE_NAMESPACE_END_DECL

#include "timer_queue_t.inl"

#endif
/*************************************************************************************/
