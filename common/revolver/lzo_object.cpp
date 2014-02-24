#include "minilzo.h"
#include "lzo_object.h"

#include <iostream>

using namespace std;

BASE_NAMESPACE_BEGIN_DECL

BaseLZObject::BaseLZObject() : wrkmem_(NULL), init_flag_(false)
{
	init();
}

BaseLZObject::~BaseLZObject()
{
	destroy();
}

void BaseLZObject::init()
{
	if (lzo_init() != LZO_E_OK)
	{
		cout << "internal error - lzo_init() failed !!!" << endl;
		return ;
	}

	init_flag_ = true;

	if(wrkmem_ == NULL)
	{
		wrkmem_ = new uint8_t[LZO_VMEM_SISE];
	}
}

void BaseLZObject::destroy()
{
	if(wrkmem_ != NULL)
	{
		delete []wrkmem_;
		wrkmem_ = NULL;
	};
}

int32_t BaseLZObject::compress(const uint8_t* src, uint32_t src_size, uint8_t *dst, uint32_t& dst_size)
{
	if(wrkmem_ == NULL || !init_flag_)
		return -1;
	
	int32_t ret = -1;
	try{
		lzo_uint out_size = dst_size;
		ret = lzo1x_1_compress(src, src_size, dst, &out_size, wrkmem_);
		if(out_size >= src_size)
			return -1;

		dst_size = out_size;
	}
	catch(...)
	{
		cout << "lzo1x_1_compress crach!!" << endl;
		return -1;
	}

	return ret;
}

int32_t BaseLZObject::uncompress(const uint8_t* src, uint32_t src_size, uint8_t *dst, uint32_t& dst_size)
{
	if(!init_flag_)
		return -1;

	int32_t ret = -1;
	try{
		lzo_uint out_size = dst_size;
		ret = lzo1x_decompress(src, src_size, dst, &out_size, NULL);
		if(ret == LZO_E_OK)
			dst_size = out_size;
	}
	catch(...)
	{
		cout << "lzo1x_decompress crach!!" << endl;
		return -1;
	}
	
	return ret;
}

BASE_NAMESPACE_END_DECL

