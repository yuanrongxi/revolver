#include "revolver/base_reactor_instance.h"
#include "core/core_packet.h"
#include "core/dc_short_conn.h"
#include "core/core_log_macro.h"
#include "core/core_local_info.h"
#include "revolver/base_sock_connector.h"
#include "core/core_bandwidth_throttler.h"

BASE_NAMESPACE_BEGIN_DECL

#define SHORT_TCP_HEARTBEAT 60000

ShortConn_Pool	SHORTCONN_POOL;

struct SShortTCPTimerParam
{
	uint32_t timer_type_;
	uint32_t delay_;
};

enum ShortTCPTimerType
{
	eShort_Connect_Timer,
	eShortTCP_Hearbeat
};

ShortConnection::ShortConnection()
{
	state_ = SHORT_CONN_IDLE;
	timer_id_ = 0;
	timer_count_ = 0;
	msg_proc_ = NULL;
	conn_id_ = 0;
}

ShortConnection::~ShortConnection()
{
	cancel_timer();
	clear_timer_events();
}

CSockStream& ShortConnection::get_sock_stream()
{
	return sock_stream_;
}

BASE_HANDLER ShortConnection::get_handle() const
{
	return sock_stream_.get_handler();
}

void ShortConnection::set_handle(BASE_HANDLER handle)
{
	sock_stream_.set_handler(handle);
}

int32_t ShortConnection::connect(const Inet_Addr& remote_addr)
{
	return connect(Inet_Addr(0, 0), remote_addr);
}

int32_t ShortConnection::connect(const Inet_Addr& src_addr, const Inet_Addr& dst_addr)
{
	CORE_DEBUG("src_addr = " << src_addr << ", TCP connect " << dst_addr );

	//打开一个SOCKET
	if(sock_stream_.open(src_addr, true, false, true) != 0)
	{
		CORE_FATAL("open tcp failed! conn = " << this);
		return -1;
	}

	CORE_DEBUG("sort conection connector ....");

	CSockConnector connector;
	if(connector.connect(sock_stream_, dst_addr) != 0)
	{
		sock_stream_.close();
		CORE_FATAL("connect failed, remote addr = " << dst_addr << ", conn = " << this);
		return -1;
	}

	state_ = SHORT_CONN_CONNECTING;
	CORE_DEBUG("short conn, state = SHORT_CONN_CONNECTING, conn = " << this);

	REACTOR_INSTANCE()->register_handler(this, MASK_WRITE);

	//设置一个连接定时器
	CORE_DEBUG("short conn set connect timer");
	timer_id_ = set_timer(eShort_Connect_Timer, 23 * 1000);

	return 0;
}

void ShortConnection::close()
{
	//删除监听事件
	REACTOR_INSTANCE()->delete_handler(this);
	//通知连接断开
	CORE_DEBUG("short conn close, conn = " << this);
	handle_close(get_handle(), MASK_TIMEOUT);
}

void ShortConnection::extern_close()
{
	//删除监听事件
	REACTOR_INSTANCE()->delete_handler(this);

	reset();
	//将连接对象返回给连接池
	SHORTCONN_POOL.push_obj(this);
	CORE_DEBUG("push short conn = " << this);
}

int32_t ShortConnection::send(CCorePacket& packet)
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

	if(sbuffer_.put(*strm))
	{
		REACTOR_INSTANCE()->register_handler(this, MASK_WRITE);
		ret = 0;
	}

	RETURN_BINSTREAM(strm);

	return ret;
}

void ShortConnection::reset()
{
	//清空到初始化状态
	sock_stream_.close();

	reactor_index_ = 0;

	state_ = SHORT_CONN_IDLE;
	CORE_DEBUG("short conn, state = SHORT_CONN_IDLE, this = " << this);

	//取消定时器
	cancel_timer();

	timer_id_ = 0;

	sbuffer_.reset();
	rbuffer_.reset();

	timer_count_ = 0;

	//释放太多的内存占用
	istrm_.rewind(true);
	istrm_.reduce();

	remote_addr_.set_ip(INADDR_ANY);
	remote_addr_.set_port(0);

	msg_proc_ = NULL;
	conn_id_ = 0;
}

uint32_t ShortConnection::set_timer(uint32_t timer_type, uint32_t delay)
{
	SShortTCPTimerParam* param = new SShortTCPTimerParam;
	param->timer_type_ = timer_type;
	param->delay_ = delay;

	return REACTOR_INSTANCE()->set_timer(this, param, delay);
}

void  ShortConnection::cancel_timer()
{
	if(timer_id_ > 0)
	{
		const void* param =NULL;
		REACTOR_INSTANCE()->cancel_timer(timer_id_, &param);
		if(param != NULL)
		{
			delete (SShortTCPTimerParam *)param;
		}

		timer_id_ = 0;
	}
}

void ShortConnection::release_timer_act(const void* act)
{
	if(act != NULL)
	{
		SShortTCPTimerParam* param = (SShortTCPTimerParam *)act;
		delete param;
	}
}

int32_t ShortConnection::handle_timeout(const void *act, uint32_t timer_id)
{
	if(timer_id_ !=  timer_id)
		return -1;

	timer_id_ = 0;

	SShortTCPTimerParam* param = (SShortTCPTimerParam *)act;
	switch(param->timer_type_)
	{
	case eShort_Connect_Timer:
		{
			delete param;

			if(state_ == SHORT_CONN_CONNECTING)
			{
				state_ = SHORT_CONN_IDLE;
				CORE_DEBUG("short conn, state = CONN_IDLE");
				this->close();
			}
		}
		break;

	case eShortTCP_Hearbeat: //心跳，PING保持
		{
			if(heartbeat() == 0)
			{
				timer_id_ = REACTOR_INSTANCE()->set_timer(this, param, SHORT_TCP_HEARTBEAT);
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

int32_t ShortConnection::handle_exception(BASE_HANDLER handle)
{
	CORE_DEBUG("handle_exception");
	if(state_ != SHORT_CONN_IDLE)
	{
		//通知服务器节点管理模块

		//通知上层连接断开
		if(msg_proc_ != NULL)
			msg_proc_->on_short_disconnected(this);

		sock_stream_.close();

		reset();
		//将连接对象返回给连接池
		SHORTCONN_POOL.push_obj(this);
		CORE_DEBUG("push short conn = " << this);
	}
	else
	{
		sock_stream_.close();
	}

	return 0;
}

int32_t ShortConnection::handle_close(BASE_HANDLER handle, ReactorMask close_mask)
{
	CORE_DEBUG("handle_close");
	if(state_ != SHORT_CONN_IDLE)
	{
		//通知上层连接断开
		if(msg_proc_ != NULL)
			msg_proc_->on_short_disconnected(this);

		sock_stream_.close();

		reset();
		//将连接对象返回给连接池
		SHORTCONN_POOL.push_obj(this);
		CORE_DEBUG("push short conn = " << this);
	}
	else
	{
		sock_stream_.close();
	}

	return 0;
}

int32_t ShortConnection::handle_input(BASE_HANDLER handle)
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

int32_t ShortConnection::handle_output(BASE_HANDLER handle)
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
			CORE_ERROR("short tcp send failed! send error, error = " << error_no() << ", conn = " << this);
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

void ShortConnection::check_connecting_state()
{
	if(state_ == SHORT_CONN_CONNECTING)
	{
		cancel_timer();

		CORE_DEBUG("short conn set heartbeat timer, conn = " << this);
		if(timer_id_ == 0)
		{
			timer_id_ = set_timer(eShortTCP_Hearbeat, SHORT_TCP_HEARTBEAT); //设置一个心跳
		}

		REACTOR_INSTANCE()->register_handler(this, MASK_READ);

		state_ = SHORT_CONN_CONNECTED;
		CORE_DEBUG("short conn, state = CONN_CONNECTED, conn = " << this);

		timer_count_ = 0;

		if(remote_addr_.get_ip() == AF_INET)
			sock_stream_.get_remote_addr(remote_addr_);

		if(msg_proc_ != NULL)
			msg_proc_->on_short_connected(this);
	}
}

int32_t ShortConnection::process(CCorePacket &packet, BinStream& istrm)
{
	uint32_t ret = 0;
	switch(packet.msg_type_)
	{
	case CORE_REQUEST:
		//消息处理
		timer_count_ = 0;

		if(msg_proc_ != NULL)
			msg_proc_->on_message(packet, istrm, this);
		break;

	case CORE_PING:
		timer_count_ = 0;
		break;
	}

	return ret; 
}

int32_t ShortConnection::heartbeat()
{
	timer_count_ ++;
	if(timer_count_ > 3) //3分钟内无任何报文，关闭连接
	{
		CORE_WARNING("short tcp connection timeout!! conn = " << this);
		this->close();

		return -1;
	}

	return 0;
}

void ShortConnection::send_ping()
{
	INIT_CORE_PING(packet);
	send(packet);

	CORE_DEBUG("short conn send ping, server = " << ", conn = " << this);
}

BASE_NAMESPACE_END_DECL

