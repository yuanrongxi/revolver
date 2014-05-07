/*************************************************************************************
*filename:	daemon_config.h
*
*to do:		定义一个所有服务器配置的接口,存储文件用JSON格式
管理
*Create on: 2013-07
*Author:	zerok
*check list:
*************************************************************************************/

#ifndef __DAEMON_CONFIG_H
#define __DAEMON_CONFIG_H

#include "core/core_daemon_event.h"

BASE_NAMESPACE_BEGIN_DECL

class CDaemonConfig : public IDaemonConfig
{
public:
	CDaemonConfig();
	virtual ~CDaemonConfig();

	void read(); 
	void write();

protected:
	string get_path();
};

BASE_NAMESPACE_END_DECL
#endif
/************************************************************************************/
