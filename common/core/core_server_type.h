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
void SetMaxServerCount(uint8_t size);

#define eClient   0x00
#define eDaemon_Server  0x01

extern const char* server_name_list[];


#endif
/************************************************************************************/

