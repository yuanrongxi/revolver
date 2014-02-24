#include "base_reactor.h"
#include "base_event_handler.h"

BASE_NAMESPACE_BEGIN_DECL

CEventHandler::CEventHandler() : reactor_index_(0)
{
	reactor_ = NULL;
}

CEventHandler::~CEventHandler()
{
}

BASE_HANDLER CEventHandler::get_handle() const
{
	return INVALID_HANDLER;
}

void CEventHandler::set_handle(BASE_HANDLER handle)
{

}

int32_t CEventHandler::handle_input(BASE_HANDLER handle)
{
	return -1;
}

int32_t CEventHandler::handle_output(BASE_HANDLER handle)
{
	return -1;
}

int32_t CEventHandler::handle_timeout(const void *act, uint32_t timer_id)
{
	return -1;
}

int32_t CEventHandler::handle_exception(BASE_HANDLER handle)
{
	return -1;
}

int32_t CEventHandler::handle_close(BASE_HANDLER handle, ReactorMask close_mask)
{
	return -1;
}

CReactor* CEventHandler::reactor() const
{
	return reactor_;
}

void CEventHandler::reactor(CReactor *r)
{
	reactor_ = r;
}

void CEventHandler::add_timer_event(uint32_t id)
{
	if(id > 0)
	{
		timer_events_.insert(id);
	}
}

void CEventHandler::del_timer_event(uint32_t id)
{
	if(id > 0)
	{
		timer_events_.erase(id);
	}
}

void CEventHandler::clear_timer_events()
{
	if(reactor_ == NULL)
		return ;

	BaseEventIDSet::iterator it = timer_events_.begin();
	uint32_t id = 0;

	while(it != timer_events_.end())
	{
		id = *it;
		it ++;

		const void* act = NULL;
		reactor_->cancel_timer(id, &act);
		if(act != NULL)
		{
			release_timer_act(act);
		}
	}

	timer_events_.clear();
	reactor_ = NULL;
}

void CEventHandler::release_timer_act(const void* act)
{

}

BASE_NAMESPACE_END_DECL
