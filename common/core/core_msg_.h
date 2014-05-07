/*************************************************************************************
*filename: core_cmd_target.h
*
*to do:		定义消息映射的一些宏
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __CORE_MSG___H
#define __CORE_MSG___H

#include "revolver/base_namespace.h"
#include "revolver/base_typedef.h"

BASE_NAMESPACE_BEGIN_DECL

enum CoreSig
{
	CoreSig_v_P_v,				// void (CBasePacket*)
	CoreSig_i_P_v,				// int32_t (CBasePacket*)
	CoreSig_i_P_u,				// int32_t (CBasePacket*, uint32_t)
	CoreSig_i_P_u_C,			// int32_t (CBasePacket*, uint32_t, CCoreConnection*)
	CoreSig_i_P_A,				// int32_t (CBasePacket*, const Inet_Addr&)

	CoreSig_Max,
};

#define ON_MSG_VOID(id, memberFxn) \
	message_map.register_callback(id, CoreSig_v_P_v, (TARGET_CALL)(static_cast< void (ICmdTarget::*)(CBasePacket*)>(memberFxn)));

#define ON_MSG_INT(id, memberFxn) \
	message_map.register_callback(id, CoreSig_i_P_v, (TARGET_CALL)(static_cast< int32_t (ICmdTarget::*)(CBasePacket*) > (memberFxn)));

#define ON_MSG_ID_INT(id, memberFxn) \
	message_map.register_callback(id, CoreSig_i_P_u, (TARGET_CALL)(static_cast< int32_t (ICmdTarget::*)(CBasePacket*, uint32_t) > (memberFxn)));

#define ON_MSG_CON_INT(id, memberFxn) \
	message_map.register_callback(id, CoreSig_i_P_u_C, (TARGET_CALL)(static_cast< int32_t (ICmdTarget::*)(CBasePacket*, uint32_t, CConnection*) > (memberFxn)));

#define ON_MSG_ADDR_INT(id, memberFxn) \
	message_map.register_callback(id, CoreSig_i_P_A, (TARGET_CALL)(static_cast< int32_t (ICmdTarget::*)(CBasePacket*, uint32_t, const Inet_Addr&)>(memberFxn)));

#define DECLARE_MSGCALL_MAP() \
protected:\
	static CTargetMessageManager* get_this_message_map(); \
	virtual CTargetMessageManager* get_message_map(); 

#define BEGIN_MSGCALL_MAP(theClass)\
	CTargetMessageManager* theClass::get_message_map() \
	{ return get_this_message_map(); } \
	CTargetMessageManager* theClass::get_this_message_map() \
	{ \
		static CTargetMessageManager message_map;\
		static bool install = true;\
		if(install)\
		{\
			install = false;

#define END_MSGCALL_MAP() \
		}\
		return &message_map;\
	};

BASE_NAMESPACE_END_DECL

#endif
