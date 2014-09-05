/*************************************************************************************
*filename:	core_daemon_client.h
*
*to do:		定义一个DC 的客户端，主要实现DC的请求管理、事务超时管理、事务参数等，用于数据库访问
*Create on: 2012-11
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __CORE_DC_CLIENT_H
#define __CORE_DC_CLIENT_H

#include "revolver/base_namespace.h"
#include "core/dc_short_conn.h"
#include "revolver/base_typedef.h"
#include "core/core_cmd_target.h"
#include "revolver/base_inet_addr.h"
#include "core/core_msg_.h"
#include "core/data_center_msg.h"

#include <set>

using namespace SERVERMESSAGE;

BASE_NAMESPACE_BEGIN_DECL

class IDCEvent;

//定义DC的回调参数模版
class CoreDCParam 
{
public:
	CoreDCParam() {conn_ = NULL; event_ = NULL; release_flag_ = false;};
	virtual ~CoreDCParam() {};
	
public:
	uint32_t			param_id_;
	ShortConnection*	conn_;
	IDCEvent*			event_;
	bool				release_flag_;
};

template<class T, uint32_t PARAM_ID>
class TypeDCParam : public CoreDCParam
{
public:
	TypeDCParam(){param_id_ = PARAM_ID; event_ = NULL;};
	virtual ~TypeDCParam(){};

	const T& data() const {return data_;};
	T&	data() {return data_;};

private:
	T data_;
};

//定义一个DC事件接口
class IDCEvent
{
public:
	IDCEvent(){};
	virtual ~IDCEvent(){};

	virtual void	dc_timeout(CoreDCParam *param, uint32_t exc_id) = 0;
	virtual void	dc_request_exception(CoreDCParam *param, uint32_t exc_id) = 0;
	virtual void	dc_response(CoreDCParam *param, const string& value, uint32_t exc_id) = 0;
	virtual void	release_param(CoreDCParam *param) = 0;
};

typedef map<uint32_t, PHPExcRequst*> DCTimerSet;

class CCoreDCClient : public CEventHandler,
					  public IShortProcessor
{
public:
	CCoreDCClient();
	virtual ~CCoreDCClient();

	void			clear();

	//提交一个PHP执行请求，ack表示是否需要回执结果，返回一个PHP执行ID
	uint32_t		post_dc_request(uint32_t dc_sid, CoreDCParam* param, const string& php, 
									const string& php_param, bool read_flag, bool ack = true);

	//取消一个PHP回执结果事件
	CoreDCParam*	cancel_dc_request(uint32_t exc_id);

	//超时消息
	int32_t			handle_timeout(const void *act, uint32_t timer_id);

	//消息处理函数
	int32_t			on_php_response(CBasePacket* packet, uint32_t sid);

	int32_t			on_short_connected(ShortConnection *conn);
	int32_t			on_short_disconnected(ShortConnection *conn);
	int32_t			on_message(CCorePacket &packet, BinStream& istrm, ShortConnection* conn);
	
protected:
	uint32_t		set_timer(CoreDCParam* param, PHPExcRequst* req, uint32_t delay = 40000);
	CoreDCParam*	cancel_timer(uint32_t timer_id);
	void			release_timer_act(const void* act);

protected:
	DCTimerSet	timer_ids_;

};

BASE_NAMESPACE_END_DECL

#endif

/************************************************************************************/


