#include <stdio.h>
#include "base_thread.h"
#include "test_tcp_delay.h"
#include "base_reactor_instance.h"
#include "base_event_handler.h"
#include "base_log.h"
#include "test_select.h"
#include "base_sock_dgram.h"
#include "base_sock_acceptor.h"
#include "base_sock_connector.h"
#include <iostream>

using namespace std;
using namespace BASE_NAMEPSACE_DECL;

struct STimerEventParam
{
	int type;
	int value; 
};

class CTimerEvent : public CEventHandler
{
public:
	CTimerEvent(){};
	virtual ~CTimerEvent(){};

	int32_t insert_timer()
	{
		STimerEventParam* param = new STimerEventParam;
		param->type = 10;
		param->value = 100;
		timer_id = REACTOR_INSTANCE()->set_timer(this, (void *)param, 1000);
		cout << "insert  timer, id = "<< timer_id << ", type = "<< param->type << ", value = " \
		<< param->value << ", delay = "<< 1000 << std::endl;

		return timer_id;
	}

	int32_t insert_timer(STimerEventParam* param)
	{
		timer_id = REACTOR_INSTANCE()->set_timer(this, (void *)param, 1000);
		cout << "insert  timer, id = "<< timer_id << ", type = "<< param->type << ", value = " \
			<< param->value << ", delay = "<< 1000 << std::endl;

		return timer_id;
	}

	virtual int32_t handle_timeout(const void *act, uint32_t timer_id)
	{
		STimerEventParam* param = (STimerEventParam *)act;
		cout << "timer id = "<< timer_id << " ,timer type = " << param->type << ", value = " << param->value << std::endl;
		//delete param;
		insert_timer(param);
		
		//发送一个UDP
		//char *data = "select udp OK";
		//Inet_Addr remote_addr("127.0.0.1", 7560);
		//send(data, strlen(data), remote_addr);
		
		return 0;
	};

	uint32_t get_timer_id() const{return timer_id;};

	int32_t	open(const Inet_Addr& local_addr)
	{
		int32_t ret = sock_dgram_.open(local_addr, true);
		if(ret == 0)
		{
			return REACTOR_INSTANCE()->register_handler(this, MASK_READ);
		}
		else
		{
			return -1;
		}
	};

	bool	close()
	{
		REACTOR_INSTANCE()->delete_handler(this);
		sock_dgram_.close();

		return true;
	}

	void send(const char* buf, int32_t size, Inet_Addr& remote_addr)
	{
		sock_dgram_.send(buf, size, remote_addr);
	}
	
	virtual BASE_HANDLER get_handle() const {return sock_dgram_.get_handler();};

	virtual int32_t handle_input(BASE_HANDLER handle)
	{
		if(handle == get_handle())
		{
			Inet_Addr remote_addr;

			char data[1024] = {0};
			int32_t size = 1024;

			if(sock_dgram_.recv(data, size, remote_addr) > 0)
			{
				cout << "recv data from " << remote_addr << ", data = " << data << endl;
			}

			return 0;
		}

		return -1;
	}

public:
	CSockDgram	sock_dgram_;
	uint32_t	timer_id;
};


class CTCPEventHandler : public CEventHandler
{
public:
	CTCPEventHandler()
	{
	}; 
	virtual ~CTCPEventHandler()
	{
	};

	CSockStream& get_sock_stream() {return sock_stream_;};
	BASE_HANDLER get_handle() const {return sock_stream_.get_handler();};
	void		 set_handle(BASE_HANDLER handle) {sock_stream_.set_handler(handle);};

	int32_t		 handle_input(BASE_HANDLER handle)
	{
		char buf[1024] = {0};
		int32_t size =1024;
		while(true)
		{
			int32_t rc = sock_stream_.recv(buf, size);
			if(rc > 0)
			{
				Inet_Addr remote_addr;
				sock_stream_.get_remote_addr(remote_addr);
				if(rc >= 4)
				{
					sock_stream_.send(buf, rc);
					//cout << "TCP recv data from " << remote_addr << ", data = " << buf << "event handler = " <<this->get_handle() <<endl;
				}
			}	
			else if(rc == 0)
			{
				cout << "rc = 0, event handler = " << this->get_handle() << endl;
				return -1;
			}
			else 
			{
				int32_t err = error_no();
				if(XEAGAIN == err || XEINPROGRESS == err)
				{
					return 0;
				}
				else
				{
					cout << "rc < 0, event handler = "<< this->get_handle() << endl;
					return -2;
				}
			}
		}
		return 0;
	}

	int32_t		handle_output(BASE_HANDLER handle)
	{
		return 0;

		cout << "CTCPEventHandler handle_output" << ", event handler = " << (uint32_t)this->get_handle() << endl;
		int32_t rc = sock_stream_.send("Select TCP OK!", strlen("Select TCP OK!"));
		if(rc > 0)
		{
			REACTOR_INSTANCE()->remove_handler(this, MASK_WRITE);	
			REACTOR_INSTANCE()->register_handler(this, MASK_READ);
		}
		else if(rc == 0)
		{
			return -1;
		}
		else
		{
			if(error_no() == XEAGAIN)
			{
				return 0;
			}
			else
			{
				return -2;
			}
		}
		return 0;
	}

	int32_t		handle_timeout(const void *act, uint32_t timer_id)
	{
		return 0;
	}

	int32_t		handle_close(BASE_HANDLER handle, ReactorMask close_mask)
	{
		sock_stream_.close();
		cout << "handle close()" << endl;
		return 0;
	}

	int32_t		handle_exception(BASE_HANDLER handle)
	{
		sock_stream_.close();
		cout << "handle_exception" << endl;
		return 0;
	}

	void send(const char *data, int32_t size)
	{
		sock_stream_.send(data, size);
	}

protected:
	CSockStream sock_stream_;
};

class CConnectHandler : public CTCPEventHandler
{
public:
	CConnectHandler(){timer_id_ = 0; connecting_ = false;};
	virtual ~CConnectHandler(){};

	int32_t connect(const Inet_Addr &server_addr) //客户端进行连接
	{
		count_ = 0;

		Inet_Addr local_addr("127.0.0.1", 0);
		sock_stream_.open(local_addr, true);

		if(connector_.connect(sock_stream_, server_addr) == 0)
		{
			REACTOR_INSTANCE()->register_handler(this, MASK_WRITE);

			STimerEventParam* param = new STimerEventParam;
			param->type = 1;
			param->value = 10;

			timer_id_ = REACTOR_INSTANCE()->set_timer(this, param, 20000);

			connecting_ = true;

			return 0;
		}
		else
		{
			return -1;
		}
	};

	int32_t			handle_input(BASE_HANDLER handle)
	{
		return 0;
	}
	/*int32_t		 handle_input(BASE_HANDLER handle)
	{
		char buf[1024] = {0};
		int32_t size =1024;
		while(true)
		{
			int32_t rc = sock_stream_.recv(buf, size);
			if(rc > 0)
			{
				Inet_Addr remote_addr;
				sock_stream_.get_remote_addr(remote_addr);
				if(rc >= 4)
				{
					uint64_t tick = 0;
					memcpy(&tick, buf, sizeof(uint64_t));
					uint64_t cur_tick = CBaseTimeValue::get_time_value().msec();
					cout << "delay = " << cur_tick - tick << "ms" << endl;
					//cout << "TCP recv data from " << remote_addr << ", data = " << buf << "event handler = " <<this->get_handle() <<endl;
				}
			}	
			else if(rc == 0)
			{
				cout << "rc = 0, event handler = " << this->get_handle() << endl;
				return -1;
			}
			else 
			{
				int32_t err = error_no();
				if(XEAGAIN == err || XEINPROGRESS == err)
				{
					return 0;
				}
				else
				{
					cout << "rc < 0, event handler = "<< this->get_handle() << endl;
					return -2;
				}
			}
		}
		return 0;
	}*/

	int32_t		handle_output(BASE_HANDLER handle)
	{
		//CBaseTimeValue::get_time_value().msec();

		count_ ++;

		uint64_t ts = CBaseTimeValue::get_time_value().msec();
		char data[1000] = {0};
		memcpy(data, &ts, sizeof(uint64_t));
		int32_t rc = sock_stream_.send(data, sizeof(uint64_t) + 200);
		//int32_t rc = sock_stream_.send("Select TCP OK!", strlen("Select TCP OK!"));
		if(rc > 0)
		{
			//cout << "rc = " << rc << ", event handler = "<<(uint32_t)this->get_handle() << endl;
			if(count_ > 100)
			{
				REACTOR_INSTANCE()->remove_handler(this, MASK_WRITE);	
			}

			REACTOR_INSTANCE()->register_handler(this, MASK_READ);

			if(timer_id_ > 0 && connecting_)
			{
				const void* param =NULL;
				REACTOR_INSTANCE()->cancel_timer(timer_id_, &param);
				timer_id_ = REACTOR_INSTANCE()->set_timer(this, param, 2000);

				connecting_ = false;
			}
		}
		else if(rc == 0)
		{
			return -1;
		}
		else
		{	
			if(error_no() == XEAGAIN)
			{
				return 0;
			}
			else
			{
				return -2;
			}
		}
		return 0;
	}

	int32_t		handle_timeout(const void *act, uint32_t timer_id)
	{
		if(connecting_)
		{
			cout << "TCP connect timeout, event handler = "<< this->get_handle() << endl;
			delete (STimerEventParam *)act;
			sock_stream_.close();
		}
		else
		{
			REACTOR_INSTANCE()->register_handler(this, MASK_WRITE);
			timer_id_ = REACTOR_INSTANCE()->set_timer(this, act, 2000);
		}

		return 0;
	};

	void close()
	{
		sock_stream_.close();
	}

protected:
	CSockConnector connector_;
	uint32_t timer_id_;

	bool connecting_;
	int32_t count_;
};

class CTCPListenHandler : public CEventHandler
{
public:
	CTCPListenHandler(){};
	virtual ~CTCPListenHandler(){};

	int32_t open(const Inet_Addr& local_addr)
	{
		if(acceptor_.open(local_addr, true, 256) == 0)
		{
			REACTOR_INSTANCE()->register_handler(this, MASK_READ);
			return 0;
		}
		else
			return -1;
	}

	int32_t close()
	{
		return acceptor_.close();
	}
		
	BASE_HANDLER get_handle() const {return acceptor_.get_socket_handler();};

	virtual int32_t handle_input(BASE_HANDLER handle)
	{
		Inet_Addr remote_addr;
		CTCPEventHandler *event = new CTCPEventHandler();
		if(acceptor_.accept(event->get_sock_stream(), remote_addr, true) == 0)
		{
			cout << "accept tcp connection, remote addr = " << remote_addr << endl;

			REACTOR_INSTANCE()->register_handler(event, MASK_WRITE | MASK_READ);	
			//REACTOR_INSTANCE()->register_handler(event, MASK_READ);
		}
		else
		{
			return -2;
		}
		return 0;
	}

	virtual int32_t handle_output(BASE_HANDLER handle)
	{
		return -1;
	}

	virtual int32_t handle_exception(BASE_HANDLER handle)
	{
		acceptor_.close();
		return 0;
	}

	virtual int32_t handle_close(BASE_HANDLER handle, ReactorMask close_mask)
	{
		acceptor_.close();
		return 0;
	}

protected:
	CSockAcceptor acceptor_;
};

class CTestSelectThread : public CThread
{
public:
	CTestSelectThread(){};
	~CTestSelectThread(){};
	
	void execute()
	{
		while(!get_terminated())
		{
			REACTOR_INSTANCE()->event_loop();
		}

		REACTOR_INSTANCE()->stop_event_loop();
		clear_thread();
	}
};

void test_select()
{
	//建立一个REACTOR反应器
	REACTOR_CREATE();
	REACTOR_INSTANCE()->open_reactor(2000);

	CTimerEvent test;

	Inet_Addr local_addr("127.0.0.1", 7560);
	test.open(local_addr);
	
	CTestSelectThread select_thr;
	select_thr.start();

	uint32_t timer_id = test.insert_timer();
	int32_t i = 0;
	while(1)
	{
		usleep(1000000);
		i ++;
		/*if(i == 100)
		{
			timer_id = test.get_timer_id();

			void* param = NULL;
			REACTOR_INSTANCE()->cancel_timer(timer_id, (const void**)&param);
			if(param != NULL)
			{
				delete (STimerEventParam*)param;
			}
		}*/

		if(i % 15 == 0)
		{
			string data = "select udp OK";
			Inet_Addr remote_addr("127.0.0.1", 7560);
			test.send(data.data(), data.size(), remote_addr);
		}
	}

	test.close();
	REACTOR_INSTANCE()->close_reactor();
}

void test_tcp_select()
{
	//建立一个REACTOR反应器
	REACTOR_CREATE();
	REACTOR_INSTANCE()->open_reactor(2000);
	CTCPListenHandler listener;
	Inet_Addr local_addr(INADDR_ANY, 6760);

	if(listener.open(local_addr) != 0)
	{
		cout << "listen failed! listen addr = " << local_addr << endl;
		return ;
	}

	CTestSelectThread select_thr;
	select_thr.start();

	/*CConnectHandler connector;
	Inet_Addr remote_addr("10.1.8.77", 1300);
	if(connector.connect(remote_addr) != 0)
	{
		cout << "connect failed! error = "<< error_no() << endl;
	}*/

	int i = 0;
	while(1)
	{
		i ++;
		usleep(1000000);

		if(i == 200)
		{
			//cout << "close" << endl;
			//connector.close();
		}
	}

	REACTOR_INSTANCE()->close_reactor();
}

#define DELAY_TCP_COUNT 20

void test_tcp_delay()
{
	//建立一个REACTOR反应器
	REACTOR_CREATE();
	REACTOR_INSTANCE()->open_reactor(2000);
	CTCPListenHandler listener;
	Inet_Addr local_addr(INADDR_ANY, 6760);

	if(listener.open(local_addr) != 0)
	{
		cout << "listen failed! listen addr = " << local_addr << endl;
		return ;
	}
	StatPacketObj	stat;
	CTestConnection tester[DELAY_TCP_COUNT];

	usleep(100000);

	CTestSelectThread select_thr;

	Inet_Addr remote_addr("192.168.60.252", 6760);
	for(int i = 0; i < DELAY_TCP_COUNT; i ++)
	{
		tester[i].connect(remote_addr);
		tester[i].set_stat_packet(&stat);
		usleep(100);
	}
	
	select_thr.start();

	while(true)
	{
		char c = getchar();
		if(c == 'e')
			break;
	}
	for(int i = 0; i < DELAY_TCP_COUNT; i ++)
	{
		tester[i].close();
	}

	REACTOR_INSTANCE()->close_reactor();
}


