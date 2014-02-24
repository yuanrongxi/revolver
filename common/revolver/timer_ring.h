/*************************************************************************************
*filename:	timer_ring.h
*
*to do:		定义定时器轮转类，实现刻度推进和轮切换
*Create on: 2012-04
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __TIMER_RING_H
#define __TIMER_RING_H

#include "base_namespace.h"
#include "base_typedef.h"

#include <set>
#include <vector>

BASE_NAMESPACE_BEGIN_DECL

using namespace std;

typedef std::set<uint32_t>		ElementSet;
typedef std::vector<ElementSet>	RingVector;

class IRingEvent
{
public:
	//轮的原子处理事件
	virtual void ring_event(uint8_t ring_id, uint32_t timer_id) = 0;
};

//时间计数环对象，默认是256个轮单元，
class CTimerRing
{
public:
	CTimerRing(uint8_t ring_id = 0);
	~CTimerRing();

	void reset();

	bool add_element(uint8_t pos, uint32_t timer_id);
	void delete_element(uint8_t pos, uint32_t timer_id);

	//scale是轮转的刻度，ring_handler是原子处理句柄,如果返回值为TRUE，说明本轮到了最末端，需要切换轮
	bool cycle(uint32_t& scale, IRingEvent* ring_handler);

	uint32_t get_pos() const {return pos_;};
	void set_pos(uint32_t pos) {pos_ = pos;};
	void set_ring_id(uint8_t id) {ring_id_ = id;};
	uint8_t get_ring_id() const {return ring_id_;};
private:
	void		clear();

private:
	uint8_t		ring_id_;		//轮子ID
	uint32_t	pos_;			//轮子指针位置

	RingVector	ring_;			//轮子
};

BASE_NAMESPACE_END_DECL
#endif
/*************************************************************************************/

