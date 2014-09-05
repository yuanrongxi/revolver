#include "core/core_connection_manager.h"
#include "revolver/base_reactor_instance.h"
#include "core/core_tcp_listener.h"
#include "core/core_log_macro.h"

BASE_NAMESPACE_BEGIN_DECL
CCoreTCPListener::CCoreTCPListener()
{
}

CCoreTCPListener::~CCoreTCPListener()
{
	this->close();
}

bool CCoreTCPListener::is_open() const
{
	return (get_handle() != INVALID_HANDLER);
}

int32_t CCoreTCPListener::open(const Inet_Addr& local_addr)
{
	if(acceptor_.open(local_addr, true) == 0)
	{
		REACTOR_INSTANCE()->register_handler(this, MASK_READ);

		CORE_INFO("open tcp listener, listener addr = " << local_addr);
		return 0;
	}
	else
	{
		CORE_ERROR("open tcp listener failed, listener addr = " << local_addr << ",error = " << error_no());
		return -1;
	}
}

int32_t CCoreTCPListener::close()
{
	REACTOR_INSTANCE()->delete_handler(this);
	acceptor_.close();
	return 0;
}

int32_t CCoreTCPListener::handle_input(BASE_HANDLER handle)
{
	while(true) //EPOLL ET模式是需要将事件全部处理干净，直到状态EAGAIN
	{
		Inet_Addr remote_addr;
		CConnection *conn = CONNECTION_POOL.pop_obj();
		conn->reset();

		if(acceptor_.accept(conn->get_sock_stream(), remote_addr, true) == 0)
		{
			CORE_INFO("accept tcp connection, remote addr = " << remote_addr);
			conn->set_remote_addr(remote_addr);

			if(REACTOR_INSTANCE()->register_handler(conn, MASK_WRITE | MASK_READ) == 0)
			{	
				conn->set_state(CConnection::CONN_CONNECTING);

				CORE_INFO("pop conn = " << conn);
			}
			else 
			{
				conn->reset();
				CONNECTION_POOL.push_obj(conn);
			}
#ifdef WIN32
			return 0;
#endif
		}
		else
		{
			conn->reset();
			CONNECTION_POOL.push_obj(conn);
#ifndef WIN32
			if (errno != EAGAIN && errno != ECONNABORTED && errno != EPROTO && errno != EINTR)
			{
				CORE_DEBUG("push conn = " << this);
				CORE_FATAL("tcp listener accept failed!");
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

int32_t CCoreTCPListener::handle_output(BASE_HANDLER handle)
{
	return 0;
}

int32_t CCoreTCPListener::handle_close(BASE_HANDLER handle, ReactorMask close_mask)
{
	return 0;
}

int32_t CCoreTCPListener::handle_exception(BASE_HANDLER handle)
{
	return 0;
}

BASE_HANDLER CCoreTCPListener::get_handle() const 
{
	return acceptor_.get_socket_handler();
};

BASE_NAMESPACE_END_DECL

