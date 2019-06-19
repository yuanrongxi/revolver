/*************************************************************************************
*filename:	lzo_object.h
*
*to do:		实现一个全局唯一的LZO的压缩对象，LZO压缩源码是GPL授权，在LINUX下可以压缩4M以下
			的报文，在WINDOW下可以压缩512K的报文！！
*Create on: 2012-05
*Author:	zerok
*check list:
*************************************************************************************/

#ifndef __LZO_OBJECT_H_
#define __LZO_OBJECT_H_

#include "base_typedef.h"
#include "base_namespace.h"
#include "base_singleton.h"

BASE_NAMESPACE_BEGIN_DECL

class BaseLZObject
{
public:
	BaseLZObject();
	~BaseLZObject();

	int32_t		compress(const uint8_t* src, uint32_t src_size, uint8_t *dst, uint32_t& dst_size);
	int32_t		uncompress(const uint8_t* src, uint32_t src_size, uint8_t *dst, uint32_t& dst_size);

private:
	void		init();
	void		destroy();

private:
	uint8_t*	wrkmem_;
	bool		init_flag_;
};

#define LZO_CREATE			CSingleton<BaseLZObject>::instance
#define LZO					CSingleton<BaseLZObject>::instance
#define LZO_DESTROY			CSingleton<BaseLZObject>::destroy

BASE_NAMESPACE_END_DECL
#endif
/************************************************************************************/
