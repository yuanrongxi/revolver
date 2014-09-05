/*************************************************************************************
*filename:	core_server_type.h
*
*to do:		定义服务器类型
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __CORE_SERVER_TYPE_H
#define __CORE_SERVER_TYPE_H

#include "revolver/base_typedef.h"
const char* GetServerName(uint8_t stype);
typedef enum EServerType
{
	eClient					= 0x00,
	eDaemon_Server			= 0x01,
	//一个例子的服务类型
	eSample_Server			= 0x02,
	//todo:可以填写其他的SERVER类型，数字必须小于0xff，而且和前面的类型不重复
	eMax_Server_Type
}EServerType;
#endif
/************************************************************************************/

