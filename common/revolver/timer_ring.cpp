#include <stdlib.h>
#include "timer_ring.h"

BASE_NAMESPACE_BEGIN_DECL
#define RING_MAX_SIZE 256
CTimerRing::CTimerRing(uint8_t ring_id) : ring_id_(ring_id)
{
	for(uint16_t i = 0; i < RING_MAX_SIZE; i ++)
	{
		ElementSet element_set;
		ring_.push_back(element_set);
	}

	pos_ = 0;
}

CTimerRing::~CTimerRing()
{
	clear();
}

void CTimerRing::clear()
{
	for(uint16_t i = 0; i < RING_MAX_SIZE; i ++)
	{
		ring_[i].clear();
	}
}

void CTimerRing::reset()
{
	pos_ = 0;
}

bool CTimerRing::add_element(uint8_t pos, uint32_t timer_id)
{
	bool ret = false;
	if(pos != pos_ || ring_id_ == 0)
	{
		ring_[pos].insert(ElementSet::value_type(timer_id));
		ret = true;
	}

	return ret;
}

void CTimerRing::delete_element(uint8_t pos, uint32_t timer_id)
{
	ring_[pos].erase(timer_id);
}

bool CTimerRing::cycle(uint32_t& scale, IRingEvent* ring_handler)
{
	if(ring_handler == NULL)
		return false;

	bool ret = false;

	uint32_t new_pos = scale + pos_;
	if(new_pos >= RING_MAX_SIZE)
	{
		new_pos = RING_MAX_SIZE;
		ret = true;

		scale = scale + pos_ - new_pos;
	}
	else 
		scale = 0;

	ElementSet::iterator it;
	for(uint16_t pos = pos_; pos < new_pos; pos ++)
	{
		for(it = ring_[pos].begin(); it != ring_[pos].end(); ++ it) //触发上一刻度的超时，有可能是中途插入的定时器
		{
			ring_handler->ring_event(ring_id_, *it);
		}

		ring_[pos].clear();

		pos_ ++;
		if(pos_ >= RING_MAX_SIZE)
		{
			pos_ = pos_ % RING_MAX_SIZE;
		}

		for(it = ring_[pos_].begin(); it != ring_[pos_].end(); ++ it)
		{
			ring_handler->ring_event(ring_id_, *it);
		}

		ring_[pos_].clear();
	}

	return ret;
}

BASE_NAMESPACE_END_DECL
