#include "core/core_cmd_target.h"
#include "core/core_packet.h"
#include "core/core_message_processor.h"
#include "core/core_message_map_decl.h"
#include "core/core_event_message.h"
#include "core/core_log_macro.h"

BASE_NAMESPACE_BEGIN_DECL
#define CORE_PACKET_HEAD_SIZE 12

#define PARSE_CORE_PACKET_TRY(packet, bin_strm, INFO_ARG) \
	try\
	{\
		bin_strm >> packet;\
	}\
	catch(...)\
	{\
		CORE_FATAL(INFO_ARG);\
		return -1;\
	}

#define PARSE_MESSAGE_BODY_TRY(body, bin_strm, INFO_ARG) \
	try \
	{\
		*bin_strm >> *body;\
	}\
	catch (...)\
	{\
		CORE_FATAL(INFO_ARG);\
		bin_strm->rewind(true);\
		bin_strm->reduce();\
		STREAMPOOL.push_obj(bin_strm);\
		return -1;\
	}

CMsgProcessor::CMsgProcessor()
{

}

CMsgProcessor::~CMsgProcessor()
{
}

void CMsgProcessor::regist_target(ICmdTarget* target)
{
	if(target == NULL)
		return;

	for(size_t i = 0; i < processors_.size(); ++i)
	{
		if(target == processors_[i])
		{
			return ;
		}
	}

	processors_.push_back(target);
}

int32_t CMsgProcessor::on_message(CCorePacket &packet, BinStream& istrm, CConnection *connection)
{
	return on_data(packet.msg_id_, packet.server_id_, istrm, connection);
}


int32_t CMsgProcessor::on_data(uint32_t msg_id, uint32_t server_id, BinStream& istrm, CConnection *connection)
{
	CBasePacket* body = MESSAGE_MAP_DECL()->gain_message(msg_id);
	if(body != NULL)
	{
		PARSE_CORE_PACKET_TRY(*body, istrm, "parse message body error!");
		for(size_t i = 0; i < processors_.size(); ++i)
		{
			if(processors_[i]->on_event(msg_id, server_id, body, connection) == 0)
				return 0;
		}
	}

	return -1;
}

int32_t CMsgProcessor::on_message(CCorePacket &packet, BinStream& istrm, const Inet_Addr& remote_addr)
{
	return on_data(packet.msg_id_, packet.server_id_,istrm, remote_addr);
}

int32_t CMsgProcessor::on_data(uint32_t msg_id, uint32_t server_id, BinStream& istrm, const Inet_Addr& remote_addr)
{
	CBasePacket* body = MESSAGE_MAP_DECL()->gain_message(msg_id);

	if(body != NULL)
	{
		PARSE_CORE_PACKET_TRY(*body, istrm, "parse message body error!");

		for(size_t i = 0; i < processors_.size(); ++i)
		{
			if(processors_[i]->on_event(msg_id, server_id, body, remote_addr) == 0)
				return 0;
		}
	}

	return -1;
}

int32_t CMsgProcessor::reciver(BinStream& bin_strm, CConnection* conn)
{
	int32_t ret = -1;
	if(bin_strm.data_size() < CORE_PACKET_HEAD_SIZE)
	{
		return ret;
	}

	CCorePacket packet;
	PARSE_CORE_PACKET_TRY(packet, bin_strm, "parse message error!");

	ret = on_message(packet, bin_strm, conn);

	return ret;
}

int32_t CMsgProcessor::reciver(BinStream& bin_strm, const Inet_Addr& remote_addr)
{
	int32_t ret = -1;
	if(bin_strm.data_size() < CORE_PACKET_HEAD_SIZE)
	{
		return ret;
	}

	CCorePacket packet;
	PARSE_CORE_PACKET_TRY(packet, bin_strm, "parse message error!");

	ret = on_message(packet, bin_strm, remote_addr);

	return ret;
}

int32_t CMsgProcessor::reciver_media(BinStream& bin_strm, CConnection* conn)
{
	int32_t ret = -1;
	//TODO:可以加入出列流媒体协议
	
	return ret;
}

int32_t CMsgProcessor::reciver_media(BinStream& bin_strm, const Inet_Addr& remote_addr)
{
	int32_t ret = -1;
	//TODO:可以加入出列流媒体协议

	return ret;
}



int32_t CMsgProcessor::on_connect(uint32_t server_id, CConnection* conn)
{
	TCP_Connect_Message msg;
	for(size_t i = 0; i < processors_.size(); ++i)//所有的消息隐射器都会受到TCP连接的事件消息
	{
		processors_[i]->on_event(TCP_CONNECT_EVENT, server_id, &msg, conn);
	}
	return 0;
}

int32_t CMsgProcessor::on_disconnect(uint32_t server_id, CConnection* conn)
{
	TCP_Close_Message msg;
	for(size_t i = 0; i < processors_.size(); ++i) //所有的消息隐射器都会受到TCP断开的事件消息
	{
		processors_[i]->on_event(TCP_CLOSE_EVENT, server_id, &msg, conn);
	}

	return -1;
}


BASE_NAMESPACE_END_DECL

