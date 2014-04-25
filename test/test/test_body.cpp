#include "test_body.h"
#include "base_guard.h"
#include "base_thread_mutex.h"
#include "object_pool.h"
#include "gettimeofday.h"
#include "base_timer_value.h"
#include "timer_node_t.h"
#include "timer_queue_t.h"
#include "base_packet.h"
#include "base_block_buffer.h"
#include "base_singleton.h"
#include "base_inet_addr.h"
#include "base_sock_dgram.h"
#include "base_thread.h"
#include "base_sock_acceptor.h"
#include "base_sock_connector.h"
#include "base_queue.h"
#include "base_log.h"
#include "base_hex_string.h"
#include "base_log_thread.h"
#include "core_server_type.h"
#include "base_cache_buffer.h"
#include "md5.h"
#include "base_file.h"
#include "json_stream.h"
#include "sil_connection_hash.h"
#include "base_nodes_load.h"

#ifdef WIN32
#include <process.h>
#else
#include <sys/wait.h>
#endif

#include <iostream>
#include <set>
#include <map>

using namespace BASE_NAMEPSACE_DECL;

BaseThreadMutex mutex;
class CGuardThread : public CThread
{
public:
	CGuardThread(){};
	~CGuardThread(){};

	 void	execute()
	 {
		 printf("guard begin\n");
		 BASE_GUARD(BaseThreadMutex, cf_mon, mutex);
		 int i = 0;
		 while(i < 15)
		 {
			 i ++;
			 usleep(100000);
		 }
		 printf("guard end\n");
	 };
};

int test_guard()
{
	CGuardThread test_thr;
	test_thr.start();

	usleep(10000);
	BASE_GUARD_RETURN(BaseThreadMutex, cf_mon1, mutex, -1);
	printf("hehe!!\n");

	int i = 0;
	while(i < 15)
	{
		usleep(100000);
		i ++;
	}
	test_thr.terminate();

	return 0;
}

class COject
{
public:
	COject(){value_= 0;};
	~COject(){};

	void set_value(int32_t value)
	{
		value_ = value;
	};

	void print() const
	{
		printf("ok, value = %d\n", value_);
	};
private:
	int value_;
};
void test_pool()
{
	ObjectMutexPool<COject, BaseThreadMutex, 100> pool;
	printf("pool size = %d\n", pool.size());

	COject* obj = pool.pop_obj();
	if(obj)
	{
		obj->set_value(100);
	}

	printf("pool size = %d\n", pool.size());
	pool.push_obj(obj);
	printf("pool size = %d\n", pool.size());
}

void test_timer_value()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	printf("sec = %d, u_sec=%d\n", (uint32_t)tv.tv_sec, (uint32_t)tv.tv_usec);

	usleep(100000);

	CBaseTimeValue timer1 = CBaseTimeValue::get_time_value();
	uint64_t ret = timer1.sec();
	//printf("sec = %d, usec = %d, msec = %d\n", (uint32_t)ret, (uint32_t)timer1.usec(), (uint32_t)timer1.msec());

	cout << "sec = " << ret <<", usec = " << timer1.usec() << ", msec = " << timer1.msec() << endl;
	usleep(1000000);
	CBaseTimeValue timer2 = CBaseTimeValue::get_time_value();
	//timer1 += timer2;
	//printf("sec = %u, u_sec=%d\n", (uint32_t)timer1.sec(), (uint32_t)timer1.usec());

	CBaseTimeValue t3 = timer2 - timer1;
	cout << "sec = " << t3.sec() << ", usec = " << t3.usec() << endl;
	//printf("sec = %u, u_sec=%d\n", (uint32_t)t3.sec(), (uint32_t)t3.usec());

	CBaseTimeValue t4 = t3 + timer1;
	cout << "sec = " << t4.sec() << ", usec = " << t4.usec() << endl;
	if(t4 == timer2)
	{
		cout << "t4 == time2" << endl;
	}

	uint64_t first = CBaseTimeValue::get_time_value().msec();
	usleep(10000);
	uint64_t second = CBaseTimeValue::get_time_value().msec();
	cout << "Sleep(1)'s delay = "<< second - first << endl;
}

void test_time_node()
{
	BaseTimerNode_T<int>	timer_node;
	CBaseTimeValue timer1(3, 1112450);
	timer_node.set_time_stamp((uint32_t)timer1.msec());

	uint8_t first = 0, second = 0, third = 0, fourth = 0;
	timer_node.get_revolver_pos(first, second, third, fourth);
	std::cout << "first = " << (uint16_t)first
		<< ", second = " << (uint16_t)second
		<<", third = " << (uint16_t)third
		<<", fourth = " << (uint16_t)fourth << std::endl;
}

struct STimerParam
{
	int type;
	int value; 
};
typedef CTimerQueue_T<CEventHandler*, CTimerFunctor, BaseThreadMutex> TIMEQUEUE;
BASE_NAMESPACE_BEGIN_DECL
class CTest_Event_Handler : public CEventHandler
{
public:
	virtual int32_t handle_timeout(const void *act, uint32_t timer_id);
	uint32_t insert_timer(CTest_Event_Handler* handler, uint32_t delay, TIMEQUEUE &tq);

	TIMEQUEUE* tq_;
};

int32_t		g_count = 0;
uint64_t	g_ts = 0;

int32_t CTest_Event_Handler::handle_timeout(const void *act, uint32_t timer_id)
{
	uint64_t ts = CBaseTimeValue::get_time_value().msec();
	STimerParam* param = (STimerParam *)act;
	//std::cout << "timer id = "<< timer_id << " ,timer type = " << param->type << ", value = " << param->value << "ts = " << ts << std::endl;
	delete param;

	++g_count;

	uint64_t cur_ts = CBaseTimeValue::get_time_value().msec();
	if(cur_ts > g_ts + 1000)
	{
		std::cout << "timer count = " << g_count << endl;
		g_ts = cur_ts;
		g_count = 0;
	}

	if(tq_ != NULL)
	{
		insert_timer(this, 240000, *tq_);
	}
	return 0;
}

uint32_t CTest_Event_Handler::insert_timer(CTest_Event_Handler* handler, uint32_t delay, TIMEQUEUE &tq)
{
	STimerParam* param = new STimerParam;
	param->type = 10;
	param->value = 10 * 10;
	uint32_t timer_id = tq.schedule(handler, (void *)param, delay, 0);
	//std:: cout << "insert  timer, id = "<< timer_id << ", type = "<< type_seed << ", value = " \
	//	<< param->value << ", delay = "<< delay << std::endl;

	return timer_id;
}
BASE_NAMESPACE_END_DECL

uint32_t type_seed = 1;
uint32_t insert_timer(CTest_Event_Handler* handler, uint32_t delay, TIMEQUEUE &tq)
{
	type_seed ++;

	STimerParam* param = new STimerParam;
	param->type = type_seed;
	param->value = type_seed * 10;
	uint32_t timer_id = tq.schedule(handler, (void *)param, delay, 0);
	//std:: cout << "insert  timer, id = "<< timer_id << ", type = "<< type_seed << ", value = " \
	//	<< param->value << ", delay = "<< delay << std::endl;

	return timer_id;
}

void test_timer_queue()
{
	srand(time(NULL));

	CTimerFunctor functor;
	TIMEQUEUE  timer_queue(&functor);
	CTest_Event_Handler handler;

	handler.tq_ = &timer_queue;

	CBaseTimeValue  begin_timer = CBaseTimeValue::get_time_value();
	for(int i = 0; i < 1000000; i ++)
	{
		insert_timer(&handler, i%240000, timer_queue);
	}
	CBaseTimeValue stop_timer = CBaseTimeValue::get_time_value();
	stop_timer = stop_timer - begin_timer;
	std::cout << "insert 1000000 timer, delay = " << stop_timer.msec() << " MS" << std::endl;

	g_ts = stop_timer.get_time_value().msec();
#if _DEBUG
	//timer_queue.set_ring_id();
#endif
	std::cout << "exprie ......" << std::endl;

	while(1)
	{
		uint32_t ms = timer_queue.expire();
		usleep((1000));
	}
}

class CTestPacket : public CBasePacket
{
public:
	void Pack(BinStream& strm) const
	{
		strm << msg_id_ << server_id_ << server_name_;
	};

	void UnPack(BinStream& strm)
	{
		strm >> msg_id_ >> server_id_ >> server_name_;
	};

	virtual void release_self()
	{

	};

	void set_value()
	{
		msg_id_ = 10;
		server_id_ = 99;
		server_name_ = "only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?only test,OK?";
	};

	void Print(std::ostream& os) const
	{
		os << "msg id = " << msg_id_ << ", server id = " << server_id_ << ", server name = " << server_name_ << endl;
	};

protected:
	uint32_t msg_id_;
	uint32_t server_id_;
	string   server_name_;
};

//将协议类的内容打包成二进制流
void test_packet()
{
	CTestPacket packet, x;
	BinStream ostrm, istrm;

	packet.set_value();

	CBasePacket* p = &x;

	ostrm << packet;

	CBaseTimeValue  begin_timer = CBaseTimeValue::get_time_value();
	//DWORD start_timer = GetTickCount();
	for(int i = 0; i < 100000; i ++)
	{
		ostrm.rewind(true);
		ostrm << packet;
	}

	//DWORD end_timer = GetTickCount();
	CBaseTimeValue stop_timer = CBaseTimeValue::get_time_value();
	stop_timer = stop_timer - begin_timer;

	cout << "encode packet count = "<< 100000 << ", delay = " << stop_timer.msec() << "ms" << endl;
}

uint8_t socket_data[10240] ={0};
uint32_t socket_data_size = 0;

class CTestSocket
{
public:

	uint32_t recv(uint8_t *buf, uint32_t size)
	{
		//CTestPacket packet;
		//packet.set_value();
		//BinStream ostrm, head;
		//ostrm << packet;
		//head << ostrm.data_size();
		//memcpy(buf, head.get_rptr(), head.data_size());
		//memcpy(buf + head.data_size(), ostrm.get_rptr(), ostrm.data_size());
		//return head.data_size() + ostrm.data_size();

		memcpy(buf, socket_data, socket_data_size);
		return socket_data_size;
	};

	uint32_t send(uint8_t *buf, uint32_t size)
	{
		socket_data_size = size;
		memcpy(socket_data, buf, size);
		return size;
	}
};

void test_block_buffer()
{
	CReciverBuffer_T<BinStream, CTestSocket, 1024 * 256> recv_buffer;
	CSenderBuffer_T<BinStream, CTestSocket, 1024 * 256> send_buffer;

	CTestSocket s;

	BinStream ostrm;
	CTestPacket packet;
	packet.set_value();
	ostrm << packet;

	send_buffer.put(ostrm);
	if(send_buffer.send(s) <= 0)
	{
		return;
	}

	uint32_t recv_size = recv_buffer.recv(s);
	if(recv_size > 0)
	{
		BinStream istrm;
		if(recv_buffer.split(istrm))
		{
			CTestPacket x;
			istrm >> x;
			cout << x;
		}
	}
}

void test_singleton()
{
	CTestPacket* p = CSingleton<CTestPacket>::instance();
	if(p != NULL)
	{
		p->set_value();
		cout << *p;
	}

	CSingleton<CTestPacket>::destroy();
}

void test_ip_addr()
{
	Inet_Addr addr("101.95.23.150", 1680);
	Inet_Addr addr2(addr);
	
	uint32_t ip = addr.get_ip();
	cout << "ip = " << addr.get_ip() << endl;

	uint8_t *value = (uint8_t *)&ip;
	cout << (uint16_t)value[0] << "." << (uint16_t)value[1] << "." << (uint16_t)value[2] << "." << (uint16_t)value[3] << endl;

	cout << "ip addr = " << addr << endl;
	cout << "ip addr2 = " << addr2 << endl;

	if(addr2 == addr)
		cout << " addr == addr2" << endl;

	Inet_Addr addr3("192.168.1.3", 1680);

	cout << "ip addr3 = " << addr3 <<endl;

	if(addr3 > addr2)
		cout << " addr3 > addr2" << endl;
	else 
		cout << " addr3 < addr2" << endl;

	BinStream ostrm;
	ostrm << addr3;

	Inet_Addr addr4;
	ostrm >> addr4;

	cout << "ip addr4 = " << addr4 << endl;
}

class CUDPRecvhread : public CThread
{
public:
	CUDPRecvhread(){};
	~CUDPRecvhread(){};

	void	execute()
	{
		Inet_Addr local_addr("127.0.0.1", 3425);
		Inet_Addr remote_addr;

		CSockDgram udp_sock;

		if(udp_sock.open(local_addr, true) == -1)
		{
			cout << "udp bind " << local_addr << " error!!!" << endl; 
		}

		char recv_buf[1024] = {0};
		int32_t buf_size  = 1024;
		while(1)
		{
			memset(recv_buf, 0x00, buf_size);
			if(udp_sock.recv(recv_buf, buf_size, remote_addr) > 0)
			{
				//cout << "recv data from " << remote_addr << ", data = " << recv_buf << endl;
			}
			else
			{
				usleep(1000);
			}
		}
	}
};

const uint8_t packet_map[16] = {
	0x00, 0x01, 0x00, 0x00, 0x48, 0xa9, 0x00, 0x01, 
	0x86, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

int test_udp()
{
	//_beginthread(sock_recv, 0, NULL);

	CUDPRecvhread recv_thr;
	recv_thr.start();

	usleep(1000000);

	Inet_Addr remote_addr("127.0.0.1", 3425);
	Inet_Addr local_addr(INADDR_ANY, 8643);
	CSockDgram udp_sock;
	
	string data = "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345";

	if(udp_sock.open(local_addr, true) == -1)
	{
		cout << "udp bind " << local_addr << " error!!!" << endl; 
		return -1;
	}
	int i = 0;
	while(1)
	{
		if(i % 3 == 0)
		{
			usleep(100000);
		}

		i++;

		uint64_t begin_ts = CBaseTimeValue::get_time_value().msec();

		for(int k = 0; k < 1000; k ++){
			udp_sock.send(data.c_str(), data.size(), remote_addr);
		}

		uint64_t cur_count = CBaseTimeValue::get_time_value().msec();
		cout << "send delay = " << cur_count - begin_ts << endl;
	}

	return 0;
}

class CTestThread : public CThread
{
public:
	CTestThread()
	{
		thr_index_ = 1;
		thr_name_ = "TestThread";
	};

	virtual ~CTestThread()
	{
		cout << "test" << endl;
	};

	void print(int i)
	{
		cout << "test, i = " << i << endl;
	};

	void print2(int i)
	{
		cout << "print2, i = " << i << endl;
	};

	void execute()
	{
		Inet_Addr local_addr("127.0.0.1", 3425);
		Inet_Addr remote_addr;

		CSockDgram udp_sock;

		if(udp_sock.open(local_addr) == -1)
		{
			cout << "udp bind " << local_addr << " error!!!" << endl; 
		}

		Inet_Addr bind_addr;
		udp_sock.get_local_addr(bind_addr);
		cout << "local addr = " << bind_addr << endl;

		char recv_buf[1024] = {0};
		int32_t buf_size  = 1024;
		while(1)
		{
			memset(recv_buf, 0x00, buf_size);
			if(udp_sock.recv(recv_buf, buf_size, remote_addr) > 0)
			{
				cout << "recv data from " << remote_addr << ", data = " << recv_buf << endl;
			}
		}
	}
};

typedef void (CThread::*AFX_PMSG)(int i);
typedef void (CTestThread::*Xi)(int i);
void test_thread()
{
	CTestThread thr;
	thr.start();

	//TODO:关于类成员函数指针的方法
	Xi p = &CTestThread::print2;
	AFX_PMSG sss = static_cast< AFX_PMSG >(p);
	CThread *ptest = &thr;
	(ptest->*sss)(10);
	cout << thr << endl;

	usleep(2000000);
	Inet_Addr remote_addr("127.0.0.1", 3425);
	Inet_Addr local_addr("127.0.0.1", 5555);

	CSockDgram udp_sock;

	string data = "1234567890";

	if(udp_sock.open(local_addr, true) == -1)
	{
		cout << "udp bind " << local_addr << " error!!!" << endl; 
		return;
	}


	while(1)
	{
		usleep(1000000);
		udp_sock.send(data.c_str(), data.size(), remote_addr);
	};
}

void send_as_packet(CSockStream& sock)
{
	CTestPacket packet;
	packet.set_value();

	BinStream ostrm, headstream;	

	char data[1024] = {0};

	ostrm << packet;

	uint32_t packet_size = ostrm.data_size();
	headstream << packet_size;
	memcpy(data, headstream.get_rptr(), headstream.data_size());

	memcpy(data + headstream.data_size(), ostrm.get_rptr(), ostrm.data_size());

	sock.send(data, headstream.data_size() + ostrm.data_size());
}

void test_as_socket()
{
	CSockAcceptor acceptor;

	Inet_Addr local_addr("127.0.0.1", 2900);
	if(acceptor.open(local_addr, false, 100) == -1)
	{
		cout << "acceptor open failed, local addr = "<< local_addr << endl;
	}
	
	char data[1024] = {0};
	CSockStream sock_strm;
	Inet_Addr remote_addr;
	string request_value = "<policy-file-request/>";
	char *policy_str = "<cross-domain-policy><site-control permitted-cross-domain-policies=\"master-only\"/><allow-access-from domain=\"*\" to-ports=\"*\"/></cross-domain-policy>\0";
	int i = 0;
	while(true)
	{	
		i ++;
		if(acceptor.accept(sock_strm, remote_addr, false) == 0)
		{
			while(true)
			{
				memset(data, 0x00, 1024);
				int32_t rc = sock_strm.recv(data, 1024);
				if(rc > 0)
				{
					if(request_value == data)
					{
						sock_strm.send(policy_str, strlen(policy_str) + 1);
						cout << "recv " << data << endl;
					}
					else
					{
						BinStream ostrm, headstream;
						headstream.resize(sizeof(uint32_t));
						headstream.set_used_size(sizeof(uint32_t));
						memcpy(headstream.get_wptr(), data, sizeof(uint32_t));

						uint32_t packet_size;
						headstream >> packet_size;

						ostrm.resize(packet_size);
						ostrm.set_used_size(packet_size);
						memcpy(ostrm.get_wptr(), data + sizeof(uint32_t), packet_size);

						CTestPacket packet;
						ostrm >> packet;

						cout << packet;

						send_as_packet(sock_strm);
					}
				}
				else
				{
					break ;
				}
			}

			sock_strm.close();
		}

		if(i == 2)
		{
			break;
		}
	}

	acceptor.close();

	cout << "exit as test" << endl;
}

class CTCPReciver : public CThread
{
public:
	CTCPReciver()
	{
		thr_index_ = 1;
		thr_name_ = "CTCPReciver";
	};

	virtual ~CTCPReciver()
	{
	};

	void execute()
	{
		CSockAcceptor acceptor;
		
		Inet_Addr local_addr(INADDR_ANY, 5555);
		if(acceptor.open(local_addr, false, 100) == -1)
		{
			cout << "acceptor open failed, local addr = "<< local_addr << endl;
		}

		CReciverBuffer_T<BinStream, CSockStream, 1024* 4> recv_buffer;

		CSockStream sock_strm;
		Inet_Addr remote_addr;
		if(acceptor.accept(sock_strm, remote_addr, false) == 0)
		{
			char data[1024] = {0};
			int32_t data_size = 1024;

			int32_t i = 0;
			while(!get_terminated())
			{
				uint32_t recv_size = recv_buffer.recv(sock_strm);
				if(recv_size > 0)
				{
					BinStream istrm;
					while(recv_buffer.split(istrm))
					{
						CTestPacket x;
						istrm >> x;
						cout << x;	

						i ++;
						cout << i << endl;
					}
				}
				else
				{
					usleep(1000);
				}
			}
			sock_strm.close();
		}

		acceptor.close();
	}
};

void test_tcp()
{
	srand((uint32_t)time(NULL));
#ifndef WIN32
	srandom((uint32_t)time(NULL));
#endif
	//char c = getchar();
	//if(c == 'r')
	{
		CTCPReciver thr;
		thr.start();

		//while(1)
		{
			usleep(100000);
		}
	}
	//else
	{
		CSockStream sock_strm;
		Inet_Addr local_addr("127.0.0.1", 20000 + rand() % 10000);
		Inet_Addr remote_addr("127.0.0.1", 5555);
		if(sock_strm.open(local_addr, false) == -1)
		{
			cout << "tcp client open failed!" << endl;
			return ;
		}

		CSenderBuffer_T<BinStream, CSockStream, 1024 * 2> send_buffer;

		BinStream ostrm;
		CTestPacket packet;
		packet.set_value();
		ostrm << packet;

		CSockConnector connector;
		if(connector.connect(sock_strm, remote_addr) != 0)
		{
			cout << "connect failed, remote addr = " << remote_addr << endl;
			return ;
		}

		//char *data = "send tcp data OK!!";
		while(1)
		{
			for(int i = 0; i < 4; i ++)
			{
				if(send_buffer.remaining_length() < ostrm.data_size() + sizeof(uint32_t))
					send_buffer.realloc_buffer(ostrm.data_size());

				send_buffer.put(ostrm);
			}

			if(send_buffer.send(sock_strm) <= 0)
			{
				return;
			}

			//sock_strm.send(data, strlen(data));
			usleep(2000000);
		}

		sock_strm.close();
	}
}

struct STestEvent
{
	int16_t		id;
	int32_t		value;
};

BaseQueue_T<STestEvent*, BaseThreadMutex, 1024>	test_q;

class CQueueThread : public CThread
{
public:
	CQueueThread()
	{
	};

	~CQueueThread()
	{
	};
	
	void execute()
	{
		STestEvent *event = NULL;
		while(1)
		{	
			event = NULL;
			if(test_q.get(event))
			{
				cout << "get event, id = " << event->id  << ", value = " << event->value << endl;
				delete event;
				event = NULL;
			}
			
			if(test_q.size() <= 0)
			{
				usleep(1000);
			}
		}
	}
};

void test_queue()
{
	CQueueThread queue_thr;
	queue_thr.start();
	int i = 0; 
	while(1)
	{
		for(int32_t in = 0; in < 3; in ++)
		{
			i ++;

			STestEvent *event = new STestEvent;
			event->id = i;
			event->value = i * 100;

			test_q.put(event);
		}

		usleep(500000);
	}
}

void test_log()
{
	LOG_CREATE();
	LOG_THREAD_CREATE();

	BaseLogStream log("test", BaseLogStream::debug);
	
	LOG_THREAD_INSTANCE()->start();

	CBaseTimeValue  begin_timer = CBaseTimeValue::get_time_value();
	for(int32_t i = 0; i < 100000; i ++)
	{	
		//BASE_GUARD(BaseThreadMutex, cf_mon, mutex);
		DEBUG_TRACE(log, "test log file, cout = " << i << ", xxx = " << i + 18);
	}	

	CBaseTimeValue stop_timer = CBaseTimeValue::get_time_value();
	stop_timer = stop_timer - begin_timer;
	cout << "write 100000 count, delay = " << stop_timer.msec() << "ms" << endl;

	LOG_THREAD_INSTANCE()->terminate();
	LOG_DESTROY();
}

void test_single_log()
{
	SingleLogStream log("test_single.log", SingleLogStream::debug);

	CBaseTimeValue  begin_timer = CBaseTimeValue::get_time_value();
	//DWORD start_timer = GetTickCount();
	for(int32_t i = 0; i < 300000; i ++)
	{	
		//BASE_GUARD(BaseThreadMutex, cf_mon, mutex);
		DEBUG_TRACE(log, "test log file, cout = " << i);
	}	

	CBaseTimeValue stop_timer = CBaseTimeValue::get_time_value();
	stop_timer = stop_timer - begin_timer;
	//DWORD stop_timer = GetTickCount();
	cout << "write 100000 count, delay = " << stop_timer.msec() << endl;
}

void test_hex_string()
{
	uint8_t data[256];
	for(int i = 0; i < 256; i++)
		data[i] = i;

	string hex = bin2asc(data, 256);
	cout << "hex = "<< hex << endl;

	uint8_t dst_data[256] = {0};
	int32_t len = 0;
	asc2bin(hex, dst_data, 256, len);

	for(int i = 0; i < len; i++)
	{
		if(i % 16 == 0)
			cout << endl;

		cout << " " << (uint16_t)dst_data[i];
	}
}
#ifndef WIN32
int fork_body()
{
	pid_t pid = fork();
	if(pid == 0)
	{
		_exit(0);
		return 0;
	}
	else
	{
		return -1;
	}
}
#endif

void test_fork()
{
	int32_t max_number = 10;
	CBaseTimeValue begin_timer = CBaseTimeValue::get_time_value();
	for(int i = 0; i < max_number; i ++)
	{
#ifndef WIN32
		fork_body();
#endif
	}

	CBaseTimeValue en_timer = CBaseTimeValue::get_time_value() - begin_timer;
	cout << "fork process delay = " << en_timer.msec() << "(ms), process number = " << max_number << endl;
}

typedef map<uint64_t, uint32_t>	TEST_MAP;
typedef list<uint32_t>	TEST_LIST;
void test_set()
{
	TEST_MAP	id_set;	

	for(uint64_t i = 0; i < 1000000; ++i)
	{
		id_set[i] = i + 100;
	}

	uint64_t first_timer = CBaseTimeValue::get_time_value().msec();

	TEST_MAP::iterator end_it = id_set.end();
	for(TEST_MAP::iterator it = id_set.begin(); it != end_it; ++it)
	{
		if(it->first != 2000)
		{
		}
	}

	uint64_t second_timer = CBaseTimeValue::get_time_value().msec();
	cout << "for delay = " << second_timer - first_timer << " ms" << endl;

}


void test_cache_buffer()
{
	CacheBuffer_T<int32_t*, 5>	cache_buffer;
/*
	int32_t max_i = 0;
	int32_t min_i = 0;

	for(int i = 0; i < 5; i ++)
	{
		int32_t* p = new int32_t;
		*p = 10 + i;
		cache_buffer.insert(*p, p);

		cout << "insert  key = " << *p << endl;

		max_i = i;
	}

	for(int32_t i = 0; i < 5; i ++)
	{
		int32_t* p = cache_buffer.find(i + 10);
		if(p != NULL)
		{
			cout << "seq = " << *p << endl;
		}
	}

	int32_t* d = cache_buffer.erase();
	if(d != NULL)
	{
		cout << "erase seq = " << *d <<endl;
		delete d;
	}
	
	min_i ++;

	for(int32_t i = 0; i < 5; i ++)
	{
		int32_t* p = cache_buffer.find(i + 10);
		if(p != NULL)
		{
			cout << "seq = " << *p << endl;
		}
	}
	
	{
		int32_t* p = new int32_t;
		*p = 10 + 6;
		for(int i = 5; i < 6; i++)
		{
			int32_t *d = cache_buffer.erase(); //删除最小的
			if(d)
				delete d;

			min_i ++;
		}

		if(!cache_buffer.insert(6, p))
		{
			delete p;
			cout << "insert  key = 6 failed!" << endl;
		}
	}

	{
		int32_t* p = new int32_t;
		*p = 10 + 5;
		cache_buffer.insert(15, p);

		cout << "insert  key = 15" << endl;
	}

	{
		int32_t* p = new int32_t;
		*p = 10 + 5;
		if(!cache_buffer.insert(15, p))
		{
			delete p;
			cout << "insert  key = 15 failed!" << endl;
		}
	}

	for(int32_t i = 0; i < 6; i ++)
	{
		int32_t* p = cache_buffer.erase();
		if(p != NULL)
		{
			cout << "erase seq = " << *p << endl;
			delete p;
		}
	}
*/
	list<int32_t *> cache_list;
	int* p = new int32_t(0);
	cache_list.push_back(p);
	p = new int32_t(2);
	cache_list.push_back(p);
	p = new int32_t(1);
	cache_list.push_back(p);
	p = new int32_t(3);
	cache_list.push_back(p);
	p = new int32_t(3);
	cache_list.push_back(p);
	p = new int32_t(4);
	cache_list.push_back(p);
	p = new int32_t(7);
	cache_list.push_back(p);
	p = new int32_t(5);
	cache_list.push_back(p);
	p = new int32_t(6);
	cache_list.push_back(p);

	int number = 0;
	while(!cache_list.empty())
	{
		p = cache_list.front();
		cache_list.pop_front();

		number ++;
		while(number > 5)
		{
			int32_t* d = cache_buffer.erase();
			if(d)
			{
				delete d;
			}

			number --;
		}

		if(!cache_buffer.insert(*p, p))
		{
			if(p)
			{
				delete p;
			}
		}
	}

	while(!cache_buffer.emtpy())
	{
		int32_t* d = cache_buffer.erase();
		if(d)
		{
			cout << "value id = " << *d << endl;
			delete d;
		}
	}
}

void test_cache_buffer2()
{
	CacheBuffer_T<int32_t*, 80>	cache_buffer;
	ObjectPool<int32_t, 100> obj_pools;

	int32_t number = 0;

	uint64_t first_timer = CBaseTimeValue::get_time_value().msec();

	for(int i = 0; i < 1000000; i ++)
	{
		int32_t* p = obj_pools.pop_obj();
		*p = i;

		number ++;
		while(number > 64)
		{
			int32_t* d = cache_buffer.erase();
			if(d)
			{
				obj_pools.push_obj(d);
			}

			number --;
		}

		if(!cache_buffer.insert(i, p))
		{
			obj_pools.push_obj(p);
		}
	}

	uint64_t second_timer = CBaseTimeValue::get_time_value().msec();
	cout << "operator delay = " << second_timer - first_timer << " ms" << endl;

	while(!cache_buffer.emtpy())
	{
		int32_t* d = cache_buffer.erase();
		if(d)
		{
			cout << "value id = " << *d << endl;
			obj_pools.push_obj(d);
		}
	}
}

void test_md5()
{
	uint64_t user_id = 52000;
	uint8_t digest[16] = {0};

	MD5_CTX md5_ctx;
	MD5Init(&md5_ctx);
	MD5Update(&md5_ctx, (unsigned char *)&user_id, sizeof(uint64_t));
	MD5Final(digest, &md5_ctx);

	string dig_str = bin2asc(digest, 16);
	cout << "dig_str = " << dig_str << endl;
}

void test_base_file()
{
	BaseFile wf, rf;
	//写文件
	if(wf.open("test.dat", "ab+"))
	{
		size_t written = 0;
		int32_t err = 0;
		uint64_t v1 = 1000;
		uint32_t v2 = 2000;
		uint32_t v3 = 1981;
		uint8_t v4 = 1;

		wf.write(&v1, sizeof(uint64_t), written, err);
		wf.write(&v2, sizeof(uint32_t), written, err);
		wf.write(&v3, sizeof(uint32_t), written, err);
		wf.write(&v4, sizeof(uint8_t), written, err);

		wf.close();
	}

	//读文件
	if(rf.open("test.dat", "rb"))
	{
		rf.set_postion(0);

		size_t read_size = 0;
		int32_t err = 0;

		uint64_t v1 = 0;
		uint32_t v2 = 0;
		int32_t v3 = 0;
		uint8_t v4 = 0;

		BaseFileResult result = SR_SUCCESS;
		while(true)
		{
			result = rf.read(&v1, sizeof(uint64_t), read_size, err);
			if(result == SR_EOS)
				break;

			result = rf.read(&v2, sizeof(int32_t), read_size, err);
			if(result == SR_EOS)
				break;

			result = rf.read(&v3, sizeof(uint32_t), read_size, err);
			if(result == SR_EOS)
				break;

			result = rf.read(&v4, sizeof(uint8_t), read_size, err);
			if(result == SR_EOS)
				break;

			cout << "v1 = " << v1 << ", v2 = " << v2 << ", v3 = " << v3 << ", v4 = " << (uint16_t)v4 << endl;
		}

		rf.close();
	}
}

using namespace JSON_NS;
class ServerInfoJson : public JsonInterface
{
public:
	ServerInfoJson(){}

	void print() const 
	{
		cout << "id = " << id << ", type = " << type << ", maxuser = " << maxUser << ", areaid = " << areaId << ", modify time = " << ModifyTime << ", modify user = " << ModifyUser << endl;
	};

protected:
	Json_Strm_T& encode(Json_Strm_T& strm, Json_Value_T& jobj)
	{
		strm.encode("id", jobj, id);
		strm.encode("type", jobj, type);
		strm.encode("maxUser", jobj, maxUser);
		strm.encode("AreaID", jobj, areaId);
		strm.encode("ModifyTime", jobj, ModifyTime);
		strm.encode("ModifyUser", jobj, ModifyUser);
		strm.encode("ip_ct", jobj, ip_ct);
		strm.encode("ip_un", jobj, ip_un);

		return strm;
	}

	Json_Strm_T& decode(Json_Strm_T& strm, Json_Value_T& jobj)
	{
		strm.decode("id", jobj, id);
		strm.decode("type", jobj, type);
		strm.decode("maxUser", jobj, maxUser);
		strm.decode("AreaID", jobj, areaId);
		strm.decode("ModifyTime", jobj, ModifyTime);
		strm.decode("ModifyUser", jobj, ModifyUser);
		strm.decode("ip_ct", jobj, ip_ct);
		strm.decode("ip_un", jobj, ip_un);

		return strm;
	}

public:
	int32_t		id;
	int32_t		type;
	int32_t		maxUser;
	int32_t		areaId;

	string		ModifyTime;
	string		ModifyUser;

	string		ip_ct;
	string		ip_un;
};

class ServerArray : public JsonInterface 
{
public:
	ServerArray() {};

protected:
protected:
	Json_Strm_T& encode(Json_Strm_T& strm, Json_Value_T& jobj)
	{
		strm.encode("SERVERS", jobj, server_list);
		return strm;
	}

	Json_Strm_T& decode(Json_Strm_T& strm, Json_Value_T& jobj)
	{
		strm.decode("SERVERS", jobj, server_list);

		return strm;
	}
public:

	vector<ServerInfoJson> server_list;
};

void test_json()
{
	ServerArray json_obj;
	string err;

	//解码JSON
	string json = "{\"SERVERS\":[{\"id\":1,\"type\":1,\"maxUser\":10,\"AreaID\":10,\"ModifyTime\":\"2013-05-13 15:15:42\",\"ModifyUser\":\"1\",\"ip_ct\":\"10.0.0.1\",\"ip_un\":\"192.168.1.1\"}]}";
	cout << "src json str = " << json << endl;
	if (json_obj.UnSerialize(json, err) != JSON_SUCCESS)
	{
		cout << "json failed!" << endl;
		return ;
	}

	for(int i = 0; i < json_obj.server_list.size(); i ++)
	{
		json_obj.server_list[i].print();
	}

	//编码JSON
	string encode_str;
	json_obj.Serialize(encode_str);
	cout << "dst json str = " << encode_str << endl;
}

class TestConn
{
public:
	TestConn(){index_ = INVALID_CONN_ID; name_ = INVALID_CONN_ID;};

	void		set_index(uint32_t index) {index_ = index;};
	uint32_t	get_index() const {return index_;};

	void		set_name(uint32_t name) {name_ = name;};
	uint32_t	get_name() const {return name_;};

	void		print()
	{
		cout << "index = " << index_ << ", name = " << name_ << endl;
	};

protected:
	uint32_t	index_;
	uint32_t	name_; 
};

class TestConnHash : public SilConnHash_T<TestConn>
{
public:
	TestConnHash() {};
	~TestConnHash(){ids_.clear();};

	void init()
	{
		for(uint32_t i = 0; i < 1000; i ++)
		{
			TestConn* conn = new TestConn();
			uint32_t index = add_conn(conn);
			conn->set_index(index);
			conn->set_name(index + 1000);

			cout << "insert conn, index = " << index << endl;

			ids_.insert(index);
		}
	};

	void destroy()
	{
		for(set<uint32_t>::iterator it = ids_.begin(); it != ids_.end(); ++ it)
		{
			TestConn* conn = find_conn(*it);
			if(conn != NULL)
			{
				del_conn(*it);
				conn->set_index(INVALID_CONN_ID);

				delete conn;
			}
		}

		ids_.clear();
	}

	void erase(uint32_t id)
	{
		TestConn* conn = find_conn(id);
		if(conn != NULL)
		{
			del_conn(id);
			conn->set_index(INVALID_CONN_ID);
		}

		ids_.erase(id);
	}

protected:
	set<uint32_t> ids_;
};

void test_conn_hash()
{
	TestConnHash conn_hash;
	conn_hash.init();

	conn_hash.erase(800);

	TestConn* conn = conn_hash.find_conn(10);
	if(conn != NULL)
	{
		conn->print();
	}

	conn_hash.destroy();
}
//服务器单元的选取，可以用此算法，能很好的平衡服务器负载问题
void test_node_load()
{
	srand((uint32_t)time(NULL));

	CNodeLoadManager load_manager;

	NodeLoadInfo node;
	for(uint32_t sid = 1; sid <= 10; ++ sid)
	{
		node.node_id = sid;
		node.node_load = rand()%90 + 10;

		cout << "sid = " << sid << ", load = " << node.node_load << endl;

		load_manager.add_node(node);
	}

	cout << "select 1000 nodes!!" << endl;
	int32_t count[10] = {0};
	for(int32_t i = 0; i < 1000; i ++)
	{
		if(load_manager.select_node(node))
			count[node.node_id - 1] ++;
	}

	for(uint32_t sid = 1; sid <= 10; sid ++)
	{
		cout << "sid = " << sid << ", count = " << count[sid - 1] << endl;
	}
}








