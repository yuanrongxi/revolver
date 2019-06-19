#include "base_bin_stream.h"
#include <stdlib.h>

BASE_NAMESPACE_BEGIN_DECL

template<class Elem, uint32_t CAPACITY>
BinStream_T<Elem, CAPACITY>::BinStream_T()
{
	//判断字节序
	union 
	{
		uint16_t	s16;
		uint8_t		s8[2];
	}un;
	un.s16 = 0x010a;
	big_endian = (un.s8[0] == 0x01);

	//开辟Binstream对象缓冲区
	size_ = sizeof(Elem) * CAPACITY;
	data_ = (Elem *)malloc(size_);

	rptr_ = (uint8_t *)data_;
	wptr_ = (uint8_t *)data_;

	used_ = 0;
	rsize_ = 0;
}

template<class Elem, uint32_t CAPACITY>
BinStream_T<Elem, CAPACITY>::~BinStream_T()
{
	if(data_ != NULL)
	{
		free(data_);
		data_ = NULL;

		rptr_ = NULL;
		wptr_ = NULL;

		size_ = 0;

		used_ = 0;
		rsize_ = 0;
	}
} 

template<class Elem, uint32_t CAPACITY>
void BinStream_T<Elem, CAPACITY>::rewind(bool reset)
{
	if(reset)
	{
		wptr_ = (uint8_t *)data_;
		used_ = 0;
	}

	rptr_ = (uint8_t *)data_;
	rsize_ = 0;
}
//重新设置内存大小
template<class Elem, uint32_t CAPACITY>
void BinStream_T<Elem, CAPACITY>::resize(uint32_t new_size)
{
	if(new_size <= size_)
		return ;

	uint32_t alloc_size  = size_;
	while(new_size >= alloc_size)
		alloc_size *= 2;

	data_ = (Elem *)realloc(data_, alloc_size);
	size_ = alloc_size;
	wptr_ = (uint8_t *)data_ + used_;
	rptr_ = (uint8_t *)data_ + 	rsize_;
}
//复位内存占用
template<class Elem, uint32_t CAPACITY>
void BinStream_T<Elem, CAPACITY>::reduce()
{
	if(size_ > 4096) //4KB
	{
		free(data_);

		size_ = sizeof(Elem) * CAPACITY;
		data_ = (Elem *)malloc(size_);

		rptr_ = (uint8_t *)data_;
		wptr_ = (uint8_t *)data_;

		used_ = 0;
		rsize_ = 0;
	}
}

BASE_NAMESPACE_END_DECL
