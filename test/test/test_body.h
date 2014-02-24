#ifndef __BASE_TEST_H
#define __BASE_TEST_H

//测试线程锁
int test_guard(); 

//测试BaseTimerValue
void test_timer_value();

//测试对象池
void test_pool();

//测试定时器
void test_time_node();
void test_timer_queue();

//测试报文对象和BinStream
void test_packet();

//测试BLOCK BUFFER模块
void test_block_buffer();

//测试单件
void test_singleton();

//测试Inet_Addr
void test_ip_addr();
//测试UDP

int test_udp();

//测试TCP
void test_tcp();

//测试线程
void test_thread();

//测试消息队列
void test_queue();

//测试日志系统
void test_log();
void test_single_log();
//一些其他的测试
void test_hex_string();
void test_fork();
void test_as_socket();
void test_set();
void test_cache_buffer();
void test_cache_buffer2();

//测试MD5
void test_md5();

//测试base file的读写
void test_base_file();

//测试JSON模块
void test_json();

//测试SilConnHash_T模块
void test_conn_hash();

//测试服务节点选取算法模块
void test_node_load();
#endif

