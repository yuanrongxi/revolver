#include "test_tcp_delay.h"

CTestConnection::CTestConnection()
{
	state_ = eTestIdle;
	timer_count_ = 0;
}

CTestConnection::~CTestConnection()
{
	cancel_timer();
}

int32_t CTestConnection::connect(const Inet_Addr& remote_addr)
{
	Inet_Addr local_addr("127.0.0.1", 0);
	sock_stream_.open(local_addr, false);
	int32_t flag = true;
	//sock_stream_.set_option(SOL_SOCKET, TCP_NODELAY, (void *)&flag, sizeof(int32_t));

	CSockConnector connector;

	if(connector.connect(sock_stream_, remote_addr) == 0)
	{
		REACTOR_INSTANCE()->register_handler(this, MASK_WRITE);
		timer_id_ = set_timer(20000);

		state_ = eTestConnecting;

		return 0;
	}
	else
	{
		sock_stream_.close();
		cout << "connect " << remote_addr << " failed!" << endl;
		return -1;
	}
}

uint32_t CTestConnection::set_timer(uint32_t delay)
{
	return REACTOR_INSTANCE()->set_timer(this, NULL, delay);
}

void CTestConnection::cancel_timer()
{
	if(timer_id_ > 0)
	{
		const void* act = NULL;
		REACTOR_INSTANCE()->cancel_timer(timer_id_, &act);
		timer_id_ = 0;
	}
}

void CTestConnection::close()
{
	REACTOR_INSTANCE()->delete_handler(this);

	sock_stream_.close();
	state_ = eTestIdle;
}

int32_t CTestConnection::handle_timeout(const void *act, uint32_t timer_id)
{
	if(timer_id_ == timer_id)
	{
		switch(state_)
		{
		case eTestIdle:
			break;

		case eTestConnecting:
			{
				cout << "connecting time out!!" << endl;
				this->close();
			}
			break;

		case eTestConnected:
			{
				timer_count_ ++;
				timer_id_ = set_timer(1000);

				//stat_->m_nTotalCount / timer_count_;
				//cout << "round send count = " << stat_->m_nTotalCount / timer_count_ << endl;
				//send();
			}
			break;

		}
	}
	return 0;
}

void CTestConnection::send()
{
	CDelayPacket packet;
	packet.ts_ = CBaseTimeValue::get_time_value().msec();
	char data[1024] = {0};
	packet.data_.resize(200);

	BinStream strm;
	strm << packet;

	if(send_buffer_.put(strm))
	{
		REACTOR_INSTANCE()->register_handler(this, MASK_WRITE);
	}
}

int32_t CTestConnection::handle_close(BASE_HANDLER handle, ReactorMask close_mask)
{
	this->close();
	return 0;
}

int32_t CTestConnection::handle_exception(BASE_HANDLER handle)
{
	this->close();
	return 0;
}

int32_t CTestConnection::handle_input(BASE_HANDLER handle)
{
	if(state_ != eTestConnected)
		return 0;

	int32_t rc = recv_buffer_.recv(sock_stream_);

	if(rc > 0)
	{
		while(recv_buffer_.split(istrm_)) //消息组装
		{
			CDelayPacket packet;

			try
			{
				istrm_ >> packet;
			}
			catch(...)
			{
				cout << "parse core packet error!!" << endl;
				return -2;
			}

			uint64_t cur_ts = CBaseTimeValue::get_time_value().msec();
			if(cur_ts > packet.ts_)
			{
				stat_->Stat(cur_ts - packet.ts_);
			//	cout << "delay = " << cur_ts - packet.ts_ << endl;
			}
			else 
			{
				stat_->Stat(1);
			}

			send();
		}
	}
	else if(rc == 0)
	{
		cout << "handle_input, socket close by peer!!" << endl;
		return -1;
	}
	else 
	{ 
		if(error_no() == XEAGAIN || error_no() == XEINPROGRESS)
		{
			return 0;
		}
		else
		{
			cout << "handle_input, recv error, error = " << error_no() << endl;
			return -2;
		}
	}

	return 0;
}

int32_t CTestConnection::handle_output(BASE_HANDLER handle)
{
	if(state_ == eTestConnecting)
	{
		cancel_timer();
		timer_id_ = set_timer(1000);

		state_ = eTestConnected;

		REACTOR_INSTANCE()->register_handler(this, MASK_READ);

		send();
	}

	if(send_buffer_.data_length() > 0 )
	{
		int32_t rc = send_buffer_.send(sock_stream_);
		if(rc == 0)
		{
			cout << "TCP send Failed! socket close by peer!!" <<endl;
			return -1;
		}
		else if(rc < 0)
		{
			if(error_no() == XEAGAIN || error_no() == XEINPROGRESS)
			{
				return 0;
			}
			else 
			{
				cout << "TCP send Failed! send error, error = " << error_no() << endl;
				return -2;
			}
		}
	}
	else if(send_buffer_.data_length() == 0) ////判断循环BUFFER中是否还有数据，如果没有，删除写入事件。
	{
		REACTOR_INSTANCE()->remove_handler(this, MASK_WRITE);
	}

	return 0;

}

