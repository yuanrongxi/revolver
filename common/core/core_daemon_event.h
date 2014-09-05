/*************************************************************************************
*filename:	core_daemon_event.h
*
*to do:		定义DAEMON事件通知接口
管理
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/

#ifndef __CORE_DAEMON_EVENT_H
#define __CORE_DAEMON_EVENT_H

#include "revolver/base_inet_addr.h"

BASE_NAMESPACE_BEGIN_DECL

class IDaemonEvent
{
public:
	IDaemonEvent(){};
	virtual ~IDaemonEvent(){};

	//其他服务器上线通知
	virtual void on_add_server(uint32_t sid, uint8_t stype, uint16_t net_type, const Inet_Addr& tel_addr, const Inet_Addr& cnc_addr){};
	//其他服务器掉线通知
	virtual void on_del_server(uint32_t sid, uint8_t stype){};
	//本地注册响应，主要是获得SID和与DAEMON通信的地址、监听端口
	virtual void on_register(uint32_t sid, uint8_t stype, uint16_t net_type, const Inet_Addr& tel_addr, const Inet_Addr& cnc_addr){};
};

//服务器配置接口，客户端可以不实现,本接口主要是实现对SERVER ID, SERVER TYPE, SERVER IP的读取和保存,主要用于保存DAEMON推送过来的信息
class IDaemonConfig
{
public:
	IDaemonConfig() {};
	virtual ~IDaemonConfig(){};

	//从配置出读取SERVER ID, SERVER TYPE, SERVER IP保存到SERVER_ID SERVER_TYPE等宏上
	virtual void read(){}; 
	virtual void write(){};
};

BASE_NAMESPACE_END_DECL
#endif	
/************************************************************************************/

