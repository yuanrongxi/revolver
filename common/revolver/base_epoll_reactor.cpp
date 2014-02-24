#include "base_event_handler.h"
#include "base_epoll_reactor.h"
#include "base_guard.h"
#include <iostream>

BASE_NAMESPACE_BEGIN_DECL

#ifndef WIN32

#define MAX_NEVENT 8192

CEpollReactor::CEpollReactor() : max_handler_num_(EPOLL_MAX_HANDLER)
, timer_queue_(&functor_), epoll_delay_(10)
{
	epfd_ = -1;	
	events_ = 0;
	for(uint32_t i = 1; i < EPOLL_HEAP_SIZE; i ++)
	{
		handler_heap_[i] = 0;
	}

	prev_ts_ = CBaseTimeValue::get_time_value().msec();

	nevent_ = 32;
}

CEpollReactor::~CEpollReactor()
{
	if(events_ != 0)
	{
		delete []events_;
		events_ = 0;
	}
}

int32_t CEpollReactor::open_reactor(uint32_t number_of_handlers)
{
	for(uint32_t i = 1; i < EPOLL_HEAP_SIZE; i ++)
	{
		handler_heap_[i] = 0;
		free_set_.insert(i);
	}

	max_handler_num_ = core_max(number_of_handlers, max_handler_num_);
	epfd_ = epoll_create(max_handler_num_);
	if(epfd_ == -1)
	{
		return -1;
	}

	events_ = new epoll_event[2 * max_handler_num_];
	return 0;
}

int32_t CEpollReactor::close_reactor()
{
	if(epfd_ != -1)
	{
		close(epfd_);
	}

	if(events_ != 0)
	{
		delete []events_;
		events_ = 0;
	}

	stop_event_loop();
	free_set_.clear();
}

ReactorEventHandlerInfo* CEpollReactor::find_handler_info(CEventHandler* handler)
{
	ReactorEventHandlerInfo* info = 0;
	uint32_t handler_index = handler->get_reactor_index();
	if(handler != 0 && handler_index < EPOLL_HEAP_SIZE &&handler_index > 0 
		&& handler_heap_[handler_index] != 0 && handler_heap_[handler_index]->event_handler == handler) 
	{
		info = handler_heap_[handler_index];
	}

	return info;
}

//检查读写后的连接是否正常
void CEpollReactor::check_connection(int32_t rc, CEventHandler* handler)
{
	if(rc < 0)
	{
		delete_handler(handler);

		handler->handle_close(handler->get_handle(), MASK_TIMEOUT);
	}
}

int32_t CEpollReactor::event_loop()
{
	if(epfd_ == -1)
	{
		return -1;
	}

	int32_t count = epoll_wait(epfd_, events_, nevent_, epoll_delay_);
	if(count < 0)
	{
		if(error_no() == EINTR)
		{
			return 0;
		}

		return -1;
	}

	CEventHandler* handler = 0;
	BASE_HANDLER handler_id = -1;

	for(register int32_t i = 0; i < count; ++i)
	{
		handler = (CEventHandler *)events_[i].data.ptr;
		if(handler == 0)
		{
			continue;
		}

		handler_id = handler->get_handle();
		
		if(events_[i].events & EPOLLIN) //读事件
		{
			int32_t rc = handler->handle_input(handler_id);

			check_connection(rc, handler);
			if(rc < 0)
			{
				continue;
			}
		}
		
		if(events_[i].events & EPOLLOUT) //写事件
		{
			int32_t rc = handler->handle_output(handler_id);

			check_connection(rc, handler);
			if(rc < 0)
			{
				continue;
			}
		}

		if((events_[i].events & EPOLLHUP) /*| (events_[i].events & EPOLLRDHUP)*/) //关闭事件
		{
			delete_handler(handler);

			handler->handle_close(handler_id, MASK_TIMEOUT);
			continue;
		}

		if(events_[i].events & EPOLLERR)//异常事件
		{
			delete_handler(handler);

			handler->handle_close(handler_id, MASK_TIMEOUT);
		}
	}

	//调整最大事件数
	if(count >= nevent_ && nevent_ < MAX_NEVENT) //8192是LIB EVENT中的最大事件数
	{
		nevent_ = 2 * nevent_;
	}

	//扫描定时器
	uint64_t cur_ts = CBaseTimeValue::get_time_value().msec();
	if(cur_ts > prev_ts_ + 5)
	{
		epoll_delay_ = timer_queue_.expire();

		//扫描内部队列
		if(msg_proc_ != 0)
		{
			msg_proc_->processor();
		}

		prev_ts_ = cur_ts;
	}
	else
		epoll_delay_ = 5;

	return 0;
}

//必须是内部消息队列触发本函数或者在REACTOR THREAD结束event_loop循环后调用
int32_t CEpollReactor::stop_event_loop()
{
	for(uint32_t i = 1; i < EPOLL_HEAP_SIZE; i ++)
	{
		if(handler_heap_[i] != 0 && handler_heap_[i]->event_handler != NULL)
		{
			handler_heap_[i]->event_handler->handle_close(handler_heap_[i]->event_handler->get_handle(), MASK_TIMEOUT);

			handler_pool_.push_obj(handler_heap_[i]);
			handler_heap_[i] = 0;
		}
	}

	free_set_.clear();

	return 0;
}

void CEpollReactor::print()
{
	for(uint32_t i = 1; i < EPOLL_HEAP_SIZE; i ++)
	{
		if(handler_heap_[i] != 0)
		{
			cout << "push ptr = " << handler_heap_[i] << ", line = " << __LINE__ << endl;
		}
	}
}

int32_t CEpollReactor::register_handler(CEventHandler *handler, uint32_t masks)
{
	if(epfd_ == -1 || handler->get_handle() == INVALID_HANDLER || free_set_.size() == 0)
	{
		return -1;
	}

	//handler->reactor(this);

	epoll_event event;
	
	event.events = EPOLLET | EPOLLHUP | EPOLLERR /*| EPOLLRDHUP*/;
	event.data.ptr = (void*)handler;

	ReactorEventHandlerInfo* info = find_handler_info(handler);
	if(info == 0)
	{
		if(masks & MASK_READ)
		{	
			event.events |= EPOLLIN ;
		}

		if(masks & MASK_WRITE)
		{
			event.events |= EPOLLOUT;
		}

		//int32_t rc = epoll_ctl(epfd_, EPOLL_CTL_ADD, handler->get_handle(), &event);
		if(epoll_ctl(epfd_, EPOLL_CTL_ADD, handler->get_handle(), &event) == -1)
		{
			return -1;
		}

		if(free_set_.empty())
		{
			return -2;
		}

		ReactorIndexSet::iterator it = free_set_.begin();

		info = handler_pool_.pop_obj();

		info->event_close_ = false;
		info->event_handler = handler;
		info->event_mask = masks;
		handler_heap_[*it] = info;
		handler->set_reactor_index(*it);

		free_set_.erase(it);
	}
	else
	{
		info->event_mask |= masks;

		if(info->event_mask & MASK_READ)
		{	
			event.events |= EPOLLIN;
		}

		if(info->event_mask & MASK_WRITE)
		{
			event.events |= EPOLLOUT;
		}

		//int32_t rc = epoll_ctl(epfd_, EPOLL_CTL_MOD, handler->get_handle(), &event);
		if(epoll_ctl(epfd_, EPOLL_CTL_MOD, handler->get_handle(), &event) == -1)
		{
			return -1;
		}
	}

	return 0;
}

int32_t CEpollReactor::remove_handler(CEventHandler *handler, uint32_t masks)
{
	if(epfd_ == -1)
	{
		return -1;
	}

	//handler->reactor(this);

	epoll_event event;

	event.events =  EPOLLET | EPOLLHUP | EPOLLERR /*| EPOLLRDHUP*/;
	event.data.ptr = (void*)handler;

	ReactorEventHandlerInfo* info = find_handler_info(handler);
	if(info != 0)
	{
		info->event_mask &= ~masks;

		if(info->event_mask & MASK_READ)
		{	
			event.events |= EPOLLIN;
		}

		if(info->event_mask & MASK_WRITE)
		{
			event.events |= EPOLLOUT;
		}

		//int32_t rc = epoll_ctl(epfd_, EPOLL_CTL_MOD, handler->get_handle(), &event);
		if(epoll_ctl(epfd_, EPOLL_CTL_MOD, handler->get_handle(), &event) == -1)
		{
			return -1;
		}
	}
	
	return 0;
}

int32_t CEpollReactor::delete_handler(CEventHandler *handler, bool del_event_obj)
{
	if(epfd_ == -1)
	{
		return -1;
	}

	ReactorEventHandlerInfo* info = find_handler_info(handler);
	if(info != 0)
	{
		handler_pool_.push_obj(info);
		handler_heap_[handler->get_reactor_index()] = 0;

		free_set_.insert(handler->get_reactor_index());
		handler->set_reactor_index(0);

		epoll_event event;
		event.events =  EPOLLHUP | EPOLLERR;
		event.data.ptr = (void *)handler;

		if(info->event_mask & MASK_READ)
		{	
			event.events |= EPOLLIN;
		}

		if(info->event_mask & MASK_WRITE)
		{
			event.events |= EPOLLOUT;
		}

		info->event_mask = 0;

		//int32_t rc = epoll_ctl(epfd_, EPOLL_CTL_DEL, handler->get_handle(), &event);
		if(epoll_ctl(epfd_, EPOLL_CTL_DEL, handler->get_handle(), &event) == -1)
		{
			return -1;
		}
	}

	return 0;
}

uint32_t CEpollReactor::set_timer(CEventHandler *event_handler, const void *act, uint32_t delay)
{
	event_handler->reactor(this);
	return timer_queue_.schedule(event_handler, act, delay, 0);
}

uint32_t CEpollReactor::cancel_timer(uint32_t timer_id, const void **act)
{
	timer_queue_.cancel_timer(timer_id, act);
	return 0;
}

#endif

BASE_NAMESPACE_END_DECL

