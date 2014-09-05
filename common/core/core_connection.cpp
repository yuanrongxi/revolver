#include "core/core_packet.h"
#include "core/core_connection.h"
#include "revolver/base_reactor_instance.h"
#include "core/core_log_macro.h"
#include "core/core_message_processor.h"
#include "revolver/base_sock_connector.h"
#include "core/core_connection_manager.h"
#include "core/core_local_info.h"
#include "core/core_bandwidth_throttler.h"
#include "revolver/md5.h"

BASE_NAMESPACE_BEGIN_DECL

//8K BUFFER
#define CLIENT_BUFFER_SIZE	8192

struct STCPTimerParam
{
	uint32_t timer_type_;
	uint32_t delay_;
};

enum TCPTimerType
{
	eConnect_Timer,
	eTCP_Hearbeat
};

CConnection::CConnection() : state_(CONN_IDLE), timer_id_(0)
{
	index_ = 0;
	server_id_ = 0;
	server_type_ = 0;
	timer_count_ = 0;

	conn_ptr_ = NULL;

	//被动方
	conn_name_ = "accept connection";

	send_flag_ = false;
}

CConnection::~CConnection()
{
	cancel_timer();
	clear_timer_events();
}

CSockStream& CConnection::get_sock_stream()
{
	return sock_stream_;
}

BASE_HANDLER CConnection::get_handle() const
{
	return sock_stream_.get_handler();
}

void CConnection::set_handle(BASE_HANDLER handle)
{
	sock_stream_.set_handler(handle);
}

void CConnection::set_state(uint16_t state)
{
	state_ = state;
	CORE_DEBUG("CConnection, state = CONN_CONNECTING");
}

void CConnection::set_conn_ptr(void* ptr)
{
	conn_ptr_ = ptr;
}

void* CConnection::get_conn_ptr()
{
	return conn_ptr_;
}

int32_t CConnection::connect(const Inet_Addr& remote_addr)
{
	return connect(Inet_Addr(0, 0), remote_addr);
}

int32_t CConnection::connect(const Inet_Addr& src_addr, const Inet_Addr& dst_addr)
{
	CORE_DEBUG("TCP connect " << dst_addr);

	//打开一个SOCKET
	if(sock_stream_.open(src_addr, true, false, true) != 0)
	{
		CORE_FATAL("open tcp failed! conn = " << this);
		return -1;
	}
	
	//主动方
	conn_name_ = "connect connection";

	CSockConnector connector;
	if(connector.connect(sock_stream_, dst_addr) != 0)
	{
		sock_stream_.close();
		CORE_FATAL("connect failed, remote addr = " << dst_addr << ", conn = " << this);
		return -1;
	}

	state_ = CONN_CONNECTING;
	CORE_DEBUG("CConnection, state = CONN_CONNECTING, conn = " << this);

	//CORE_DEBUG("CConnection, register MASK_WRITE");
	REACTOR_INSTANCE()->register_handler(this, MASK_WRITE);

	//设置一个连接定时器
	CORE_DEBUG("set connect timer");
	timer_id_ = set_timer(eConnect_Timer, 10 * 1000);

	return 0;
}

void CConnection::close()
{
	//删除监听事件
	REACTOR_INSTANCE()->delete_handler(this);
	//关闭SOCKET
	//sock_stream_.close();
	//通知连接断开
	CORE_DEBUG("connection close, conn = " << this);
	handle_close(get_handle(), MASK_TIMEOUT);
}

void CConnection::extern_close()
{
	//删除监听事件
	REACTOR_INSTANCE()->delete_handler(this);

	reset();
	//将连接对象返回给连接池
	CONNECTION_POOL.push_obj(this);
	CORE_DEBUG("push conn = " << this);
}

int32_t CConnection::send(CCorePacket& packet, bool no_delay)
{
	int32_t ret = -1;

	GAIN_BINSTREAM(strm);

	*strm << packet;

	if(sbuffer_.remaining_length() < strm->data_size() + sizeof(uint32_t))
	{
		if(sbuffer_.length() < MAX_BUFFER_SIZE)//扩大TCP发送缓冲区,防止缓冲区太小造成发送包异常
		{
			sbuffer_.realloc_buffer(strm->data_size() + sizeof(uint32_t));
			CORE_WARNING("sbuffer realloc buffer, size = " << sbuffer_.length());
		}
		else //发送报文丢弃
		{
			CORE_ERROR("sbuffer is full, sbuffer.size = " << sbuffer_.length());
			RETURN_BINSTREAM(strm);

			return -1;
		}
	}

	THROTTLER()->add_tcp_packet(strm->data_size(), true);

	send_flag_ = true;

	if(sbuffer_.put(*strm))
	{
		if(no_delay)
		{
			sbuffer_.send(sock_stream_);
		}


		if(sbuffer_.data_length() > 0)
		{
			REACTOR_INSTANCE()->register_handler(this, MASK_WRITE);
		}
		else
		{
			REACTOR_INSTANCE()->remove_handler(this, MASK_WRITE);
		}

		ret = 0;
	}

	RETURN_BINSTREAM(strm);

	return ret;
}

int32_t CConnection::send(const string& bin_stream)
{
	if(bin_stream.empty())
		return -1;

	uint32_t data_size = bin_stream.size();

	if(sbuffer_.remaining_length() < data_size + sizeof(uint32_t))
	{
		if(sbuffer_.length() < MAX_BUFFER_SIZE)//扩大TCP发送缓冲区,防止缓冲区太小造成发送包异常
		{
			sbuffer_.realloc_buffer(data_size + sizeof(uint32_t));
			CORE_WARNING("sbuffer realloc buffer, size = " << sbuffer_.length());
		}
		else //发送报文丢弃
		{
			CORE_ERROR("sbuffer is full, sbuffer.size = " << sbuffer_.length());
			return -1;
		}
	}

	send_flag_ = true;

	THROTTLER()->add_tcp_packet(data_size, true);

	if(sbuffer_.put(bin_stream))
	{
		if(sbuffer_.data_length() > 0)
		{
			REACTOR_INSTANCE()->register_handler(this, MASK_WRITE);
		}
		else
		{
			REACTOR_INSTANCE()->remove_handler(this, MASK_WRITE);
		}
	}

	return 0;
}

void CConnection::reset()
{
	//清空到初始化状态
	sock_stream_.close();

	reactor_index_ = 0;

	state_ = CONN_IDLE;
	CORE_DEBUG("CConnection, state = CONN_IDLE, this = " << this);

	//取消定时器
	cancel_timer();

	timer_id_ = 0;

	sbuffer_.reset();
	rbuffer_.reset();

	index_ = 0;
	server_id_ = 0;
	server_type_ = 0;

	timer_count_ = 0;

	conn_ptr_ = NULL;

	send_flag_ = false;

	//释放太多的内存占用
	istrm_.rewind(true);
	istrm_.reduce();

	remote_addr_.set_ip(INADDR_ANY);
	remote_addr_.set_port(0);
}

uint32_t CConnection::set_timer(uint32_t timer_type, uint32_t delay)
{
	STCPTimerParam* param = new STCPTimerParam;
	param->timer_type_ = timer_type;
	param->delay_ = delay;

	return REACTOR_INSTANCE()->set_timer(this, param, delay);
}

void  CConnection::cancel_timer()
{
	if(timer_id_ > 0)
	{
		const void* param = NULL;
		REACTOR_INSTANCE()->cancel_timer(timer_id_, &param);
		if(param != NULL)
		{
			delete (STCPTimerParam *)param;
		}

		timer_id_ = 0;
	}
}

void CConnection::release_timer_act(const void* act)
{
	STCPTimerParam* param = (STCPTimerParam *)act;
	if(param != NULL)
	{
		delete param;
	}
}

int32_t CConnection::handle_timeout(const void *act, uint32_t timer_id)
{
	if(timer_id_ !=  timer_id)
		return -1;

	timer_id_ = 0;

	STCPTimerParam* param = (STCPTimerParam *)act;
	switch(param->timer_type_)
	{
	case eConnect_Timer:
		{
			delete param;

			if(state_ == CONN_CONNECTING)
			{				
				CONN_MANAGER()->on_del_connection(this);

				//通知上层连接断开
				MSG_PROCESSOR()->on_disconnect(server_id_, this);

				state_ = CONN_IDLE;
				CORE_DEBUG("CConnection, state = CONN_IDLE");
				this->close();
			}
		}
		break;

	case eTCP_Hearbeat: //心跳，PING保持
		{
			if(heartbeat() == 0)
			{
				timer_id_ = REACTOR_INSTANCE()->set_timer(this, param, 60000);
			}
			else
			{
				delete param;
			}
		}
		break;
	}

	return 0;
}

int32_t CConnection::handle_exception(BASE_HANDLER handle)
{
	CORE_DEBUG("handle_exception, error = " << error_no());
	if(state_ != CONN_IDLE)
	{
		//通知服务器节点管理模块
		CONN_MANAGER()->on_del_connection(this);

		//通知上层连接断开
		MSG_PROCESSOR()->on_disconnect(server_id_, this);
		
		sock_stream_.close();

		reset();
		//将连接对象返回给连接池
		CONNECTION_POOL.push_obj(this);
		CORE_DEBUG("push conn = " << this);
	}
	else
	{
		sock_stream_.close();
	}
	
	return 0;
}

int32_t CConnection::handle_close(BASE_HANDLER handle, ReactorMask close_mask)
{
	CORE_DEBUG("handle_close");
	if(state_ != CONN_IDLE)
	{
		//通知服务器节点管理模块
		CONN_MANAGER()->on_del_connection(this);

		//通知上层连接断开
		MSG_PROCESSOR()->on_disconnect(server_id_, this);
		
		sock_stream_.close();

		reset();
		//将连接对象返回给连接池
		CONNECTION_POOL.push_obj(this);
		CORE_DEBUG("push conn = " << this);
	}
	else
	{
		sock_stream_.close();
	}

	return 0;
}

int32_t CConnection::handle_input(BASE_HANDLER handle)
{
	CCorePacket	recv_packet;

	while(true) 
	{
		recv_packet.body_ptr_ = NULL;

		if(rbuffer_.remaining_length() == 0) //扩大TCP接收缓冲区,防止缓冲区太小造成收包异常
		{
			if(rbuffer_.length() < MAX_BUFFER_SIZE)
				rbuffer_.realloc_buffer(rbuffer_.length() - 1);
			else
			{
				CORE_ERROR("rbuffer_ is full, rbuffer_.size = " << rbuffer_.length());
				return -2;
			}
		}

		int32_t rc = rbuffer_.recv(sock_stream_);

		if(rc > 0)
		{
			THROTTLER()->add_tcp_packet(rc, false);
			
			bool split_flag = true;
			while(split_flag) //消息组装
			{
				int32_t split_ret = rbuffer_.split(istrm_);
				if(split_ret == 0)
				{
					split_flag = true;

					try{ istrm_ >> recv_packet;}
					catch(...)
					{
						CORE_FATAL("parse core packet error!!");
						return -2;
					}

					if(process(recv_packet, istrm_) != 0)
						return 0;
				}
				else if(split_ret < 0)
					return -1;
				else //表示包合法，继续收！
				{
#ifndef WIN32
					split_flag = false;
#else
					return 0;
#endif
				}
			}
		}
		else if(rc == 0)
		{
			CORE_ERROR("handle_input, socket close by peer!! conn = " << this);
			return -1;
		}
		else 
		{ 
			if(error_no() == XEAGAIN || error_no() == XEINPROGRESS || error_no() == XEINTR)
			{
				return 0;
			}
			else
			{
				CORE_ERROR("handle_input, recv error, error = " << error_no() << ", conn = " << this);
				return -2;
			}
		}
	}

	return 0;
}

int32_t CConnection::handle_output(BASE_HANDLER handle)
{
	check_connecting_state();

	//发送数据
	int32_t rc = sbuffer_.send(sock_stream_);
	if(rc < 0)
	{
		if(error_no() == XEAGAIN || error_no() == XEINPROGRESS)
		{
			return 0;
		}
		else 
		{
			CORE_ERROR("TCP send Failed! send error, error = " << error_no() << ", conn = " << this);
			return -2;
		}
	}

	//判断循环BUFFER中是否还有数据，如果没有，删除写入事件。
	if(sbuffer_.data_length() == 0)
	{
		REACTOR_INSTANCE()->remove_handler(this, MASK_WRITE);
	}

	return 0;
}

void CConnection::check_connecting_state()
{
	if(state_ == CONN_CONNECTING)
	{
		cancel_timer();

		CORE_DEBUG("set heartbeat timer, conn = " << this);
		if(timer_id_ == 0)
		{
			timer_id_ = set_timer(eTCP_Hearbeat, 60000); //设置一个心跳
		}

		REACTOR_INSTANCE()->register_handler(this, MASK_READ);

		state_ = CONN_CONNECTED;
		CORE_DEBUG("CConnection, state = CONN_CONNECTED, conn = " << this);

		timer_count_ = 0;

		//主动连接方，发送握手消息
		send_handshake();

		if(remote_addr_.get_ip() == AF_INET)
			sock_stream_.get_remote_addr(remote_addr_);
	}
}

int32_t CConnection::process(CCorePacket &packet, BinStream& istrm)
{
	uint32_t ret = 0;
	switch(packet.msg_type_)
	{
	case CORE_REQUEST:
		//消息处理
		MSG_PROCESSOR()->on_message(packet, istrm, this);
		timer_count_ = 0;
		break;

	case CORE_MEDIA_SHELL:
		break;

	case CORE_PONG:
		timer_count_ = 0;
		break;

	case CORE_PING:
		process_ping(packet);
		break;

	case CORE_HANDSHAKE:
		ret = process_handshake(packet, istrm);
		break;
	}

	return ret; 
}

int32_t CConnection::process_handshake(const CCorePacket &packet, BinStream& istrm)
{
	server_id_ = packet.server_id_;
	server_type_ = packet.server_type_;

	//对服务器进行校验
	/*if(server_type_ > eClient && SERVER_TYPE > eClient)
	{
		HandShakeBody body;
		try{
			istrm >> body;
			string digest_data;
			generate_digest(server_id_, server_type_, digest_data);
			//握手非法摘要
			if( digest_data != body.digest_data)
			{
				CORE_WARNING("unsuited server connection!!");
				this->close();
				return -1;
			}
		}
		catch(...)
		{
			CORE_FATAL("parse core packet error!!");
			this->close();
			return -2;
		}
	}*/

	//可能未触发第一个MASK_WRITE
	check_connecting_state();

	//通知服务器节点管理模块
	CONN_MANAGER()->on_add_connection(this);

	CORE_DEBUG("process TCP HANDSHAKE, conn = " << this);

	//通知上层有新的连接进入，被动连接方
	MSG_PROCESSOR()->on_connect(server_id_, this);

	timer_count_ = 0;

	return 0;
}

void CConnection::process_ping(const CCorePacket &packet)
{
	timer_count_ = 0;
	CORE_DEBUG("recv ping, conn = " << this << "sid = " << server_id_ << ", server = " << GetServerName(server_type_));
	if(server_type_ > eClient && SERVER_TYPE > eClient)
	{
		INIT_CORE_PONG(pong);
		send(pong);
	}
}

int32_t CConnection::heartbeat()
{
	timer_count_ ++;
	//CORE_INFO("heartbeat, timer_count_ = " << timer_count_ << ", conn = " << this);
	if(timer_count_ > 4) //4分钟内无任何报文，关闭连接
	{
		CORE_WARNING("tcp connection timeout!! conn = " << this);
		this->close();

		return -1;
	}
	else
	{
		//调整分配的内存，防止客户端连接内存过大
		buffer_reduce();
		send_ping();

		send_flag_ = false;
	}

	return 0;
}

void CConnection::buffer_reduce()
{
	//如果数据长度为0，而且是对客户端的连接
	if(server_type_ == eClient && sbuffer_.length() > CLIENT_BUFFER_SIZE && sbuffer_.data_length() == 0)
		sbuffer_.reset();

	//如果本地是客户端，进行接收BUFFER的调整
	if(SERVER_TYPE == eClient)
	{
		if(rbuffer_.length() > CLIENT_BUFFER_SIZE && rbuffer_.data_length() == 0)
			rbuffer_.reset();

		istrm_.reduce();
	}
}

void CConnection::send_ping()
{
	if(!send_flag_)
	{
		INIT_CORE_PING(packet);
		send(packet);

		CORE_DEBUG("send ping, server = " << GetServerName(server_type_) << ", conn = " << this);
	}
}

void CConnection::send_handshake()
{
	CORE_DEBUG("send TCP HANDSHAKE, conn = " << this);
	INIT_CORE_HANDSHAKE(packet);
	HandShakeBody body;
	if(server_type_ > eClient)
		generate_digest(SERVER_ID, SERVER_TYPE, body.digest_data);

	packet.set_body(body);

	send(packet);

	send_flag_ = false;
}

void CConnection::generate_digest(uint32_t server_id, uint8_t server_type, string& digest_data)
{
	digest_data.clear();

	//产生一个握手MD5摘要
	uint8_t digest[MD5_DIGEST_LENGTH] = {0};

	MD5Context md5_ctx;
	MD5Init(&md5_ctx);

	MD5Update(&md5_ctx, (unsigned char *)&(server_id), sizeof(uint32_t));
	MD5Update(&md5_ctx, (unsigned char *)&(server_type), sizeof(uint8_t));
	MD5Update(&md5_ctx, (unsigned char *)(SERVER_CERFITY.data()), SERVER_CERFITY.size());
	MD5Final(digest, &md5_ctx);

	digest_data.assign((char *)digest, MD5_DIGEST_LENGTH);
}


BASE_NAMESPACE_END_DECL
