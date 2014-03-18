// Revolver.cpp : Defines the entry point for the console application.
//
#include "base_os.h"
#include "test_body.h"
#include "test_select.h"
#include "core_test.h"
#include <time.h>
#include "gettimeofday.h"
#include "base_log.h"

#include <iostream>
using namespace std;

void test_localtime_s()
{
	struct tm tm_now;
	struct timeval tv;
	gettimeofday(&tv, NULL);

	time_t now = tv.tv_sec;

#ifdef WIN32
	::localtime_s(&tm_now, &now);
#else
	::localtime_r(&now, &tm_now);
#endif

	char data[1024] = {0};

	sprintf(data, "%02d:%02d:%02d.%3ld", tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec, tv.tv_usec /1000);
	cout << data << endl;
}


class CBaseTest
{
public:
	CBaseTest(){};
	virtual ~CBaseTest(){};

	virtual void XX() = 0;
	void print()
	{
		XX();
	};
};

class CTest : public CBaseTest
{
public:
	CTest(){};
	~CTest() {};

	virtual void XX() 
	{
		cout << "OOOOO!!!!" <<endl;
	};
};

int main(int argc, char* argv[])
{
	init_socket();
	//test_guard();
	//test_timer_value();
	//test_pool();
	//test_time_node();
	//test_timer_queue();
	//test_packet();
	//test_block_buffer();
	//test_singleton();
	//test_ip_addr();
	test_udp();
	//test_thread();
	//test_tcp();
	//test_queue();
	//test_localtime_s();
	//test_log();
	//test_single_log();
	//test_select();
	//test_tcp_select();
	//test_tcp_delay();
	//test_message_call();
	//test_message_call2();
	//test_hex_string();
	//test_core_udp();
	//test_core_tcp_server();
	//test_fork();
	//test_as_socket();
	//test_set();
	//test_cache_buffer();
	//test_cache_buffer2();
	//destroy_socket(); 
	//test_md5();
	//test_base_file();
	//test_json();
	//test_conn_hash();
	//test_node_load();

	LOG_DESTROY(); //·ÀÖ¹LOGÄÚ´æÐ¹Â¶
	return 0;
}

