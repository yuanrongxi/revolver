/*************************************************************************************
*filename: core_message_map_decl.h
*
*to do:		定义CORE的内部消息体与消息ID之间的映射关系
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __CORE_MESSAGE_MAP_DECL_H
#define __CORE_MESSAGE_MAP_DECL_H

#ifdef WIN32
#include <hash_map>
#else
#include <ext/hash_map>
using namespace __gnu_cxx;
#endif

#include "revolver/base_packet.h"
#include "revolver/base_singleton.h"

BASE_NAMESPACE_BEGIN_DECL

typedef hash_map<uint32_t, CBasePacket*>		CoreMessageMap;

class CMessageMapDecl
{
public:
	CMessageMapDecl();
	~CMessageMapDecl();
	
	//获取一个消息体句柄
	CBasePacket*	gain_message(uint32_t msg_id);
	//注册一个消息体句柄
	void			register_message(uint32_t msg_id, CBasePacket* msg_obj);

private:
	void			clear();

private:
	CoreMessageMap	msg_map_;
};

#define CREATE_MESSAGE_MAP_DECL		CSingleton<CMessageMapDecl>::instance
#define MESSAGE_MAP_DECL			CSingleton<CMessageMapDecl>::instance
#define DESTROY_MESSAGE_MAP_DECL	CSingleton<CMessageMapDecl>::destroy

//定义映射加载宏
#define MESSAGEMAP_DECL_BEGIN(entity)\
class CMessageMapDecl_##entity	\
{\
public:\
	static void decl_msg_map() \
	{
#define MESSAGEMAP_REGISTER(id,body)\
		MESSAGE_MAP_DECL()->register_message(id,body);

#define MESSAGEMAP_DECL_END()\
	} \
};

#define LOAD_MESSAGEMAP_DECL(entity) \
	CMessageMapDecl_##entity::decl_msg_map();

#define LOAD_MESSAGEMAP_DECL2(e1, e2) \
	CMessageMapDecl_##e1::decl_msg_map();\
	CMessageMapDecl_##e2::decl_msg_map();

#define LOAD_MESSAGEMAP_DECL3(e1, e2, e3) \
	CMessageMapDecl_##e1::decl_msg_map();\
	CMessageMapDecl_##e2::decl_msg_map();\
	CMessageMapDecl_##e3::decl_msg_map();

#define LOAD_MESSAGEMAP_DECL4(e1, e2, e3, e4) \
	CMessageMapDecl_##e1::decl_msg_map();\
	CMessageMapDecl_##e2::decl_msg_map();\
	CMessageMapDecl_##e3::decl_msg_map();\
	CMessageMapDecl_##e4::decl_msg_map();

BASE_NAMESPACE_END_DECL
#endif

/************************************************************************************/
