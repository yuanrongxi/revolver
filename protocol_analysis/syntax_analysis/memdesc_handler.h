/*
 * 处理类型和消息定义的成员描述
 */
#ifndef MEMDESC_HANDLER_H_
#define MEMDESC_HANDLER_H_

#include "basedefine.h"

class CMemDescHandler
{
public:
	static CMemItem CheckMemDesc(string& str_line, int line_num);

	//检查v_mem中是否有与str_name重名的变量定义
	static bool		IsMemNameNotUsed(string& str_name, MemItemVec& v_mem, int line_num);
};

#endif