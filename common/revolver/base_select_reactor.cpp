#include "base_event_handler.h"
#include "base_select_reactor.h"
#include "base_guard.h"

BASE_NAMESPACE_BEGIN_DECL

#define ERASE_HANDLER \
	handler_map_.erase(it ++);\
	handler_pool_.push_obj(info);\
	continue\

CSelectReator::CSelectReator() : timer_queue_(&functor_)
{
	max_handler_num_ = 64;
	select_delay_ = 20;
}

CSelectReator::~CSelectReator()
{
}

int32_t CSelectReator::open_reactor(uint32_t number_of_handlers)
{
	if(number_of_handlers < max_handler_num_)
	{
		max_handler_num_ = number_of_handlers;
	}

	return 0;
}

int32_t CSelectReator::close_reactor()
{
	return 0;
}

int32_t CSelectReator::event_loop()
{
	BASE_HANDLER max_fd = 0;
	if(handler_map_.empty())
	{
		usleep(select_delay_ * 1000);
	}
	else
	{
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
		FD_ZERO(&expeption_set);
		vector<BASE_HANDLER> closed_handlers;

		ReactorEventHandlerMap::iterator it;
		//进行SELECT监听
		for(ReactorEventHandlerMap::iterator it = handler_map_.begin(); it != handler_map_.end();)
		{
			ReactorEventHandlerInfo *info = it->second;
			if(info->event_close_)
			{
				ERASE_HANDLER;
			}

			BASE_HANDLER fd = info->event_handler->get_handle();
			if(fd == INVALID_HANDLER)
			{
				it ++;
				info->event_handler->handle_close(fd, MASK_TIMEOUT);

				continue;
			}

			if((info->event_mask & MASK_READ) == MASK_READ)
			{
				FD_SET(fd, &read_set);
			}

			if((info->event_mask & MASK_WRITE) == MASK_WRITE)
			{
				FD_SET(fd, &write_set);
			}

			FD_SET(fd, &expeption_set);

			if(fd > max_fd)
				max_fd = fd;

			++it;
		}

		max_fd ++;
	}

	if(max_fd != 0)
	{
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = select_delay_ * 1000;

		int32_t count = ::select(max_fd, &read_set, &write_set, &expeption_set, &timeout);
		if(count >= 0)
		{
			for(ReactorEventHandlerMap::iterator it = handler_map_.begin(); it != handler_map_.end();)
			{
				ReactorEventHandlerInfo *info = it->second;

				if(info->event_close_)
				{
					++it;
					continue;
				}

				BASE_HANDLER fd = info->event_handler->get_handle();

				if(FD_ISSET(fd, &expeption_set))
				{
					info->event_handler->handle_exception(fd);
					ERASE_HANDLER;
				}

				if((info->event_mask & MASK_READ) == MASK_READ && FD_ISSET(fd, &read_set))
				{
					int32_t rc = info->event_handler->handle_input(fd);
					if(rc == -1)
					{
						info->event_handler->handle_close(fd, MASK_TIMEOUT);
						ERASE_HANDLER;
					}
					else if(rc == -2)
					{
						info->event_handler->handle_exception(fd);
						ERASE_HANDLER;
					}
				}

				if((info->event_mask & MASK_WRITE) == MASK_WRITE && FD_ISSET(fd, &write_set))
				{
					int32_t rc = info->event_handler->handle_output(fd);
					if(rc == -1)	//连接关闭
					{
						info->event_handler->handle_close(fd, MASK_TIMEOUT);
						ERASE_HANDLER;
					}
					else if(rc == -2) //连接异常
					{
						info->event_handler->handle_exception(fd);
						ERASE_HANDLER;
					}
				}

				++ it;
			}
		}
	}

	//扫描定时器
	select_delay_ = timer_queue_.expire();
	//扫描内部队列
	if(msg_proc_ != NULL)
	{
		msg_proc_->processor();
	}

	return 0;
}

int32_t CSelectReator::stop_event_loop()
{
	for(ReactorEventHandlerMap::iterator it = handler_map_.begin(); it != handler_map_.end();)
	{
		ReactorEventHandlerInfo *info = it->second;
		ERASE_HANDLER;
	}

	return 0;
}

int32_t CSelectReator::register_handler(CEventHandler *handler, uint32_t masks)
{
	if(handler == NULL || handler->get_handle() == INVALID_HANDLER)
		return -1;

	//handler->reactor(this);

	ReactorEventHandlerInfo *info = NULL;
	ReactorEventHandlerMap::iterator it = handler_map_.find(handler->get_handle());
	if(it == handler_map_.end())
	{
		info = handler_pool_.pop_obj();

		info->event_handler = NULL;
		info->event_mask = 0;
		info->event_close_ = false;

		handler_map_[handler->get_handle()] = info;
	}
	else
	{
		info = it->second;
	}

	info->event_close_ = false;
	info->event_handler = handler;
	info->event_mask |= masks;

	return 0;
}

int32_t CSelectReator::remove_handler(CEventHandler *handler, uint32_t masks)
{
	if(handler == NULL)
		return -1;

	//handler->reactor(this);
	ReactorEventHandlerMap::iterator it = handler_map_.find(handler->get_handle());
	if(it != handler_map_.end())
	{
		ReactorEventHandlerInfo* info = it->second;
		info->event_mask &= ~masks;
	}

	return 0;
}

int32_t CSelectReator::delete_handler(CEventHandler *handler, bool del_event_obj /* = false */)
{
	if(handler == NULL)
		return -1;

	ReactorEventHandlerMap::iterator it = handler_map_.find(handler->get_handle());
	if(it != handler_map_.end())
	{
		FD_CLR(handler->get_handle(), &write_set);
		FD_CLR(handler->get_handle(), &read_set);
		FD_CLR(handler->get_handle(), &expeption_set);

		it->second->event_close_ = true;
	}

	return 0;
}

uint32_t CSelectReator::set_timer(CEventHandler *event_handler, const void *act, uint32_t delay)
{
	event_handler->reactor(this);
	return timer_queue_.schedule(event_handler, act, delay, 0);
}

uint32_t CSelectReator::cancel_timer(uint32_t timer_id, const void **act)
{
	timer_queue_.cancel_timer(timer_id, act);
	return 0;
}

BASE_NAMESPACE_END_DECL
