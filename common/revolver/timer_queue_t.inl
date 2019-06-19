/////////////////////////////////////////////////////////////////////////////////////////
#include "timer_queue_t.h"
#include "base_guard.h"
#include <iostream>

BASE_NAMESPACE_BEGIN_DECL

#define UNINT32_MAX  4294967296

#define SELECT_DELAY 5

/////////////////////////////////////////////////////////////////////////////////////////

template<class HANDLER, class FUNCTOR, class LOCK>
CTimerQueue_T<HANDLER, FUNCTOR, LOCK>::CTimerQueue_T(FUNCTOR* functor, size_t heap_size /* = 10240 */)
{
	heap_size_ = 0;

	alloc_heap(heap_size);

	used_num_ = 0;
	cur_heap_pos_ = 0;
	//获得起始时间
	start_time_ = CBaseTimeValue::get_time_value();
	prev_time_ = start_time_;

	functor_ = functor;

	for(uint8_t index = 0; index < RINGS_SIZE; index ++)
	{
		rings_[index].set_ring_id(index);
	}
}

template<class HANDLER, class FUNCTOR, class LOCK>
CTimerQueue_T<HANDLER, FUNCTOR, LOCK>::~CTimerQueue_T()
{
	clear();
}

template<class HANDLER, class FUNCTOR, class LOCK>
void CTimerQueue_T<HANDLER, FUNCTOR, LOCK>::clear()
{
	BASE_GUARD(LOCK, cf_mon, mutex_);

	for(uint32_t i = 0; i < heap_size_; i ++)
	{
		if(heap_[i] != NULL)
		{
			node_pool_.push_obj(heap_[i]);
			heap_[i] = NULL;
			freeTimers_.push_back(i);
		}
	}

	used_num_ = 0;
	cur_heap_pos_ = 0;
}

template<class HANDLER, class FUNCTOR, class LOCK>
bool CTimerQueue_T<HANDLER, FUNCTOR, LOCK>::full() const
{
	return used_num_ >= heap_size_ - 1;
}

template<class HANDLER, class FUNCTOR, class LOCK>
void CTimerQueue_T<HANDLER, FUNCTOR, LOCK>::alloc_heap(size_t size)
{
	heap_.resize(size + heap_size_);
	for(uint32_t i = 0; i < size; i ++)
	{
		heap_[heap_size_] = NULL;
		if(heap_size_  > 0)
			freeTimers_.push_back(heap_size_);

		heap_size_ ++;
	}
}

template<class HANDLER, class FUNCTOR, class LOCK>
uint32_t CTimerQueue_T<HANDLER, FUNCTOR, LOCK>::get_free_node()
{
	if(full())
	{
		alloc_heap(heap_size_ + 1);
	}

	uint32_t ret = freeTimers_.front();
	freeTimers_.pop_front();
	if(heap_[ret] != NULL)
		assert(0);

	return ret;
}

template<class HANDLER, class FUNCTOR, class LOCK>
FUNCTOR &CTimerQueue_T<HANDLER, FUNCTOR, LOCK>::upcall_functor()
{
	return *(this->functor_);
}

template<class HANDLER, class FUNCTOR, class LOCK>
uint32_t CTimerQueue_T<HANDLER, FUNCTOR, LOCK>::schedule(HANDLER handler, 
																		   const void *act, 
																		   uint32_t delay, 
																		   uint32_t interval)
{
	BASE_GUARD_RETURN(LOCK, cf_mon, mutex_, 0);

	BaseTimerNode_T<HANDLER>* timer_obj = node_pool_.pop_obj();
	if(timer_obj != NULL)
	{
		uint32_t timer_id = get_free_node();
		
		CBaseTimeValue cur_timer = CBaseTimeValue::get_time_value();

		uint64_t distance = delay; //直接以当前时间作为坐标，相差一个扫描间隔20MS
		if(cur_timer > start_time_)
			distance = (cur_timer.msec() - start_time_.msec() + delay);// SELECT_DELAY;

		distance = distance % (UNINT32_MAX);
		
		timer_obj->set(handler, act, (uint32_t)(core_max(distance, 1)), interval, timer_id);

		heap_[timer_id] = timer_obj;

		used_num_ ++;

		insert_node(timer_obj);

		upcall_functor().registration(timer_obj->get_handler(), timer_id);

		return timer_id;
	}

	return 0;
}

template<class HANDLER, class FUNCTOR, class LOCK>
void CTimerQueue_T<HANDLER, FUNCTOR, LOCK>::cancel_timer(uint32_t timer_id, const void **act)
{
	BASE_GUARD(LOCK, cf_mon, mutex_);
	if(timer_id < heap_size_ && heap_[timer_id] != NULL)
	{
		BaseTimerNode_T<HANDLER>* timer_obj = heap_[timer_id];
		delete_node(timer_obj);

		heap_[timer_id] = NULL;
		if(used_num_ > 0)
			used_num_ --;

		freeTimers_.push_back(timer_id);

		*act = timer_obj->get_act();
		upcall_functor().cancel_timer(timer_obj->get_handler(), timer_id);

		node_pool_.push_obj(timer_obj);
	}
}

template<class HANDLER, class FUNCTOR, class LOCK>
uint32_t CTimerQueue_T<HANDLER, FUNCTOR, LOCK>::reset_timer(uint32_t timer_id, uint32_t delay, uint32_t interval /* = 0 */)
{
	BASE_GUARD_RETURN(LOCK, cf_mon, mutex_, 0);

	if(timer_id < heap_size_ && heap_[timer_id] != NULL)
	{
		BaseTimerNode_T<HANDLER>* timer_obj = heap_[timer_id];
		delete_node(timer_obj);

		timer_obj->set_internal(interval);

		CBaseTimeValue cur_timer = CBaseTimeValue::get_time_value();
		uint64_t distance = delay; // SELECT_DELAY; //直接以当前时间作为坐标，相差一个扫描间隔20MS
		if(cur_timer > start_time_)
			distance = (cur_timer.msec() - start_time_.msec() + delay); // SELECT_DELAY;

		distance = distance % (UNINT32_MAX);
		timer_obj->set_time_stamp(core_max(distance, 1));

		insert_node(timer_obj);

		return timer_id;
	}
	else
	{
		return 0;
	}
}

template<class HANDLER, class FUNCTOR, class LOCK>
uint32_t CTimerQueue_T<HANDLER, FUNCTOR, LOCK>::expire()
{
	BASE_GUARD_RETURN(LOCK, cf_mon, mutex_, 0);
	uint32_t ret = SELECT_DELAY;	//默认20MS

	CBaseTimeValue cur_timer = CBaseTimeValue::get_time_value();
	
	if(cur_timer > prev_time_)
	{
		uint32_t scale = static_cast<uint32_t>((cur_timer.msec() - prev_time_.msec()));// SELECT_DELAY);
		if(scale > 0)
		{
			ret = revolver(scale);
			prev_time_ = cur_timer;
		}
	}

	return ret;
}

template<class HANDLER, class FUNCTOR, class LOCK>
uint32_t CTimerQueue_T<HANDLER, FUNCTOR, LOCK>::revolver(uint32_t scale)
{
	//std::cout << "pos, first = " << rings_[0].get_pos() << ", second = " << rings_[1].get_pos()
	//	<< ", third = " << rings_[2].get_pos() << ", fourth = " << rings_[3].get_pos() <<std::endl;

	uint32_t ret = SELECT_DELAY;

	uint8_t index = 0;
	uint32_t rewind_scale = scale;
	while(rewind_scale > 0)
	{
		index = 0;
		if(rings_[index].cycle(rewind_scale, this)) //扫描第一轮
		{
			index ++;
			uint32_t sc = 1;
			while(rings_[index].cycle(sc, this))//扫描下一轮，刻度只往前推进1格
			{
				sc = 1;
				index ++;
				if(index >= RINGS_SIZE)
				{
					start_time_ = CBaseTimeValue::get_time_value();
					break;
				}
			}
		}
	}

	return ret;
}

template<class HANDLER, class FUNCTOR, class LOCK>
void CTimerQueue_T<HANDLER, FUNCTOR, LOCK>::ring_event(uint8_t ring_id, uint32_t timer_id)
{
	if(heap_[timer_id] == NULL)
		return;

	BaseTimerNode_T<HANDLER>* timer_obj = heap_[timer_id];
	if(ring_id == 0)
	{
		//超时通知
		BaseTimerDispathInfo_T<HANDLER> info;
		if(timer_obj != NULL)
		{
			timer_obj->get_dispatch_info(info);
			this->dispatch_info(info, timer_id);

			node_pool_.push_obj(timer_obj);
		}

		heap_[timer_id] = NULL;
		if(used_num_ > 0)
			used_num_ --;

		freeTimers_.push_back(timer_id);
	}
	else if(ring_id < RINGS_SIZE)
	{
		//移动RING的位置，例如，第二轮子的某个单元的所有定时器映射到第一轮子上进行重新分布
		insert_node(timer_obj);
	}
}

template<class HANDLER, class FUNCTOR, class LOCK>
void CTimerQueue_T<HANDLER, FUNCTOR, LOCK>::insert_node(BaseTimerNode_T<HANDLER>* node)
{
	uint32_t timer_id = node->get_timer_id();

	uint8_t poss[RINGS_SIZE] = {0};
	node->get_revolver_pos(poss[RINGS_SIZE - 1], poss[RINGS_SIZE - 2], poss[RINGS_SIZE - 3], poss[RINGS_SIZE - 4]);
	//cout << "timer id = "<< timer_id << ", first = " << (uint16_t)poss[RINGS_SIZE - 4] << ",second = " << (uint16_t)poss[RINGS_SIZE - 3] 
	//<< ",third = " << (uint16_t)poss[RINGS_SIZE - 2] <<", fouth = " << (uint16_t)poss[RINGS_SIZE - 1] <<endl;
	uint8_t index = RINGS_SIZE - 1;
	while(!rings_[index].add_element(poss[index], timer_id))
	{
		if(index == 0)
			break ;

		index --;
	}
}

template<class HANDLER, class FUNCTOR, class LOCK>
void CTimerQueue_T<HANDLER, FUNCTOR, LOCK>::delete_node(BaseTimerNode_T<HANDLER>* node)
{
	uint32_t timer_id = node->get_timer_id();
	uint8_t poss[RINGS_SIZE] = {0};
	node->get_revolver_pos(poss[RINGS_SIZE - 1], poss[RINGS_SIZE - 2], poss[RINGS_SIZE - 3], poss[RINGS_SIZE - 4]);

	for(uint8_t index = 0; index < RINGS_SIZE; index ++) //在每个轮上进行删除
	{
		rings_[index].delete_element(poss[index], timer_id);
	}
}

template<class HANDLER, class FUNCTOR, class LOCK>
void CTimerQueue_T<HANDLER,  FUNCTOR, LOCK>::dispatch_info(BaseTimerDispathInfo_T<HANDLER>& info, uint32_t timer_id)
{
	upcall_functor().timer_out(info.handler_, info.act_, info.recurring_, timer_id);
}

#if _DEBUG
template<class HANDLER, class FUNCTOR, class LOCK>
void CTimerQueue_T<HANDLER,  FUNCTOR, LOCK>::set_ring_id()
{
	rings_[1].set_pos(255);
	rings_[2].set_pos(0);
	rings_[3].set_pos(0);
}
#endif

BASE_NAMESPACE_END_DECL
/////////////////////////////////////////////////////////////////////////////////////////

