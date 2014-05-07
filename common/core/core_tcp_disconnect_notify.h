/*************************************************************************************
*filename: core_tcp_disconnect_notify.h
*
*to do:		节点掉线通告接口，可以用作服务器停止服务判断
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __CORE_TCP_DISCONNECT_NOTIFY_H
#define __CORE_TCP_DISCONNECT_NOTIFY_H

#include "revolver/base_namespace.h"
#include "revolver/base_typedef.h"

BASE_NAMESPACE_BEGIN_DECL

class ICoreServerNotify
{
public:
	//DAEMON和本地服务器与此服务器的连接都断了
	virtual void on_server_dead(uint32_t sid, uint8_t stype) = 0;
};

BASE_NAMESPACE_END_DECL

#endif
/************************************************************************************/
