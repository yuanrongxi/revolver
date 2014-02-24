#ifndef __TEST_SELECT_H
#define __TEST_SELECT_H

//测试reactor反应器,windows下用的是select，linux下用的是epoll et
void test_select();
void test_tcp_select();

void test_tcp_delay();

#endif
