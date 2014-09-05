/*************************************************************************************
*filename: core_message_processor.h
*
*to do:		CORE的消息内部处理器,消息分发器，分发各个消息模块进行
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __CORE_MESSAGE_PROCESSOR_H
#define __CORE_MESSAGE_PROCESSOR_H

#include "revolver/base_namespace.h"
#include "revolver/base_typedef.h"
#include "revolver/base_singleton.h"
#include "core/core_reciver_interface.h"

#include <vector>
using namespace std;

BASE_NAMESPACE_BEGIN_DECL

class CCorePacket;
class ICmdTarget;
class CConnection;

typedef vector<ICmdTarget *> Processor_Array;

class CMsgProcessor : public IReciver
{
public:
	CMsgProcessor();
	~CMsgProcessor();

	void regist_target(ICmdTarget* target);
	//所有TCP消息入口
	int32_t on_message(CCorePacket &packet, BinStream& istrm, CConnection *connection);
	//所有UDP消息入口
	int32_t on_message(CCorePacket &packet, BinStream& istrm, const Inet_Addr& remote_addr);

	//处理UDP消息
	int32_t reciver(BinStream& bin_strm, const Inet_Addr& remote_addr);
	//处理TCP消息
	int32_t reciver(BinStream& bin_strm,  CConnection* conn);

	int32_t	reciver_media(BinStream& bin_strm,  const Inet_Addr& remote_addr);
	int32_t	reciver_media(BinStream& bin_strm, CConnection* conn);

	//TCP断开消息
	int32_t on_connect(uint32_t server_id, CConnection* conn);
	int32_t on_disconnect(uint32_t server_id, CConnection* conn);

protected:
	int32_t	on_data(uint32_t msg_id, uint32_t server_id, BinStream& istrm,  CConnection *connection);
	int32_t on_data(uint32_t msg_id, uint32_t server_id, BinStream& istrm, const Inet_Addr& remote_addr);

	//消息引射器数组尽量保持在32个以下
	Processor_Array	processors_;
};

#define CREATE_MSG_PROCESSOR	CSingleton<CMsgProcessor>::instance
#define MSG_PROCESSOR			CSingleton<CMsgProcessor>::instance
#define DESTROY_MSG_PROCESSOR	CSingleton<CMsgProcessor>::destroy

#define ADD_PROCESSOR(PROC) \
	MSG_PROCESSOR()->regist_target(PROC)

//创建并初始化消息分发器,X,Y,Z是主要的消息处理类，例如：DAEMON消息处理类
#define INIT_MSG_PROCESSOR1(X) \
	CREATE_MSG_PROCESSOR();\
	ADD_PROCESSOR(X);

#define INIT_MSG_PROCESSOR2(X, Y) \
	CREATE_MSG_PROCESSOR();\
	ADD_PROCESSOR(X);\
	ADD_PROCESSOR(Y);

#define INIT_MSG_PROCESSOR3(X, Y, Z) \
	CREATE_MSG_PROCESSOR();\
	ADD_PROCESSOR(X);\
	ADD_PROCESSOR(Y);\
	ADD_PROCESSOR(Z);

BASE_NAMESPACE_END_DECL
#endif
/************************************************************************************/
