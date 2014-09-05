#ifndef __BASE_REACTOR_INSTANCE_H
#define __BASE_REACTOR_INSTANCE_H

#include "revolver/base_select_reactor.h"
#include "revolver/base_epoll_reactor.h"

#ifdef WIN32
#define REACTOR_CREATE	 CSingleton<CSelectReator>::instance
#define REACTOR_INSTANCE CSingleton<CSelectReator>::instance
#define REACTOR_DESTROY  CSingleton<CSelectReator>::destroy
#else
#define REACTOR_CREATE	 CSingleton<CEpollReactor>::instance
#define REACTOR_INSTANCE CSingleton<CEpollReactor>::instance
#define REACTOR_DESTROY  CSingleton<CEpollReactor>::destroy
#endif

#endif
