/*************************************************************************************
*filename: core_cmd_target.h
*
*to do:		消息映射基础类
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __CORE_CMD_TARGET_H
#define __CORE_CMD_TARGET_H

#include "revolver/base_namespace.h"
#include "revolver/base_typedef.h"
#include "revolver/base_inet_addr.h"
#include <map>

using namespace std;

BASE_NAMESPACE_BEGIN_DECL
class CBasePacket;
class CConnection;

struct CMD_MESSAGE_ENTRY;
class CTargetMessageManager;

class ICmdTarget
{
public:
	ICmdTarget();
	virtual ~ICmdTarget();

	//定义各种触发参数,例如：SID,mssage class, connection句柄等等
	virtual int32_t on_event(uint32_t msg_id, uint32_t sid, CBasePacket* packet, CConnection* connection);
	//处理UDP消息
	virtual int32_t on_event(uint32_t msg_id, uint32_t sid, CBasePacket* packet, const Inet_Addr& remote_addr);

protected:
	virtual CTargetMessageManager* get_message_map() = 0;
};

typedef void (ICmdTarget::*TARGET_CALL)(void);
typedef map<uint32_t, CMD_MESSAGE_ENTRY>	CMD_MESSAGE_MAP;

typedef struct CMD_MESSAGE_ENTRY
{
	uint32_t	msg_id;		//消息
	uint32_t	sig;		//函数格式
	TARGET_CALL	fun;		//触发函数
}CMD_MESSAGE_ENTRY;

//不用数组做回调映射，否则消息多的话，效率就有问题
class CTargetMessageManager
{
public:
	CTargetMessageManager();
	~CTargetMessageManager();

	void		register_callback(uint32_t msg_id, uint32_t sig, TARGET_CALL fun);

	CMD_MESSAGE_ENTRY* get_callback(uint32_t msg_id);

protected:
	CMD_MESSAGE_MAP	message_map_;
};

BASE_NAMESPACE_END_DECL

#endif
/************************************************************************************/
