/*************************************************************************************
*filename:	timer_node_t.h
*
*to do:		定义定时器单元模版，实现对定时器参数的管理
*Create on: 2012-04
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __TIMER_NODE_T_H
#define __TIMER_NODE_T_H

#include "base_namespace.h"
#include "base_typedef.h"

BASE_NAMESPACE_BEGIN_DECL

template<class HANDLER>
class BaseTimerDispathInfo_T
{
public:
	HANDLER		handler_;		//触发句柄
	const void* act_;			//定时器参数
	int32_t		recurring_;		//是否循环定时
};

template <class HANDLER>
class BaseTimerNode_T
{
public:
	BaseTimerNode_T();
	~BaseTimerNode_T();

	void set(HANDLER handler, const void* act, uint32_t timeout_stamp, uint32_t intval, uint32_t timer_id);

	HANDLER&	get_handler()
	{
		return handler_;
	};

	void		set_handler(HANDLER& handler) 
	{
		handler_ = handler;
	};

	const void*	get_act() const
	{
		return act_;
	};

	void		set_act(void* act)
	{
		act_ = act;
	};

	uint32_t	get_internal() const
	{
		return internal_;
	};

	void		set_internal(uint32_t internal)
	{
		internal_ = internal;
	};

	uint32_t	get_timer_id() const
	{
		return timer_id_;
	};

	void		set_timer_id(uint32_t timer_id)
	{
		timer_id_ = timer_id;
	}

	void		set_time_stamp(uint32_t stamp)
	{
		timeout_stamp_ = stamp;
	}

	uint32_t	get_time_stamp() const
	{
		return timeout_stamp_;
	}

	void		get_dispatch_info(BaseTimerDispathInfo_T<HANDLER> &info);

	//获取轮转的位置
	void		get_revolver_pos(uint8_t& first, uint8_t &second, uint8_t& third, uint8_t& fourth) const;
private:
	HANDLER			handler_;
	const void*		act_;
	uint32_t		timeout_stamp_;	//超时时间戳，MS为单位刻度
	uint32_t		internal_;		//循环定时的时间间隔
	uint32_t		timer_id_;		//定时器ID
};


BASE_NAMESPACE_END_DECL

#include "timer_node_t.inl"

#endif		
/*************************************************************************************/
