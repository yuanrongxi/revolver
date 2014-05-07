#include "revolver/base_reactor_instance.h"
#include "core/dc_short_conn.h"
#include "dc_tcp_listener.h"
#include "core/core_log_macro.h"

BASE_NAMESPACE_BEGIN_DECL

DCTCPListener::DCTCPListener()
{
	proc_ = NULL;
}

DCTCPListener::~DCTCPListener()
{
	this->close();
}

bool DCTCPListener::is_open() const
{
	return (get_handle() != INVALID_HANDLER);
}

int32_t DCTCPListener::open(const Inet_Addr& local_addr)
{
	if(acceptor_.open(local_addr, true) == 0)
	{
		REACTOR_INSTANCE()->register_handler(this, MASK_READ);

		CORE_INFO("open short tcp listener, listener addr = " << local_addr);
		return 0;
	}
	else
	{
		CORE_ERROR("open short tcp listener failed, listener addr = " << local_addr << ",error = " << error_no());
		return -1;
	}
}

int32_t DCTCPListener::close()
{
	REACTOR_INSTANCE()->delete_handler(this);
	acceptor_.close();
	return 0;
}

int32_t DCTCPListener::handle_input(BASE_HANDLER handle)
{
	while(true)
	{
		Inet_Addr remote_addr;
		ShortConnection *conn = SHORTCONN_POOL.pop_obj();
		conn->reset();

		if(acceptor_.accept(conn->get_sock_stream(), remote_addr, true) == 0)
		{
			CORE_INFO("accept tcp short connection, remote addr = " << remote_addr);
			conn->set_remote_addr(remote_addr);
			conn->set_proc(proc_); //管理消息处理句柄

			REACTOR_INSTANCE()->register_handler(conn, MASK_WRITE);	
			REACTOR_INSTANCE()->register_handler(conn, MASK_READ);

			conn->set_state(ShortConnection::SHORT_CONN_CONNECTING);

			CORE_INFO("pop short conn = " << conn);

#ifdef WIN32
			return 0;
#endif
		}
		else
		{
			SHORTCONN_POOL.push_obj(conn);
#ifndef WIN32
			if (errno != EAGAIN && errno != ECONNABORTED && errno != EPROTO && errno != EINTR)
			{
				CORE_DEBUG("push short conn = " << this);
				CORE_FATAL("short tcp listener accept failed!");
				return -2;
			}
			else 
			{
				return 0;
			}
#else 
			return -2;
#endif
		}
	}

	return 0;
}

int32_t DCTCPListener::handle_output(BASE_HANDLER handle)
{
	return 0;
}

int32_t DCTCPListener::handle_close(BASE_HANDLER handle, ReactorMask close_mask)
{
	return 0;
}

int32_t DCTCPListener::handle_exception(BASE_HANDLER handle)
{
	return 0;
}

BASE_HANDLER DCTCPListener::get_handle() const 
{
	return acceptor_.get_socket_handler();
};

BASE_NAMESPACE_END_DECL

