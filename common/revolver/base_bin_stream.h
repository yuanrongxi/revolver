/*************************************************************************************
*filename:	base_bin_stream.h
*
*to do:		实现协议对象的二进制打包流对象,网络字节序模式流
*Create on: 2012-04
			2012-11 增加 =号重载、bin_to_string两个函数，主要实现对STRING的相互转换
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __BASE_BIN_STREAM_H
#define __BASE_BIN_STREAM_H

#include "revolver/base_namespace.h"
#include "revolver/base_typedef.h"
#include "revolver/base_hex_string.h"
#include <assert.h>
#include <string.h>
#include <string>

#define MAX_STREAM_STIRNG_SIZE 10485760 //10M

using namespace std;

BASE_NAMESPACE_BEGIN_DECL

//1K的默认大小
#define DEFAULT_PACKET_SIZE	 1024

#define ENCODE(data, type) \
	int32_t type_size = sizeof(type);\
	resize(used_ + type_size);\
	uint8_t* pos = (uint8_t*)&data;\
	if(big_endian) \
	{\
		::memcpy(wptr_, pos, type_size);\
	}\
	else\
	{\
		pos = pos + type_size - 1;\
		for(int32_t i = 0; i < type_size; ++i)\
		{\
			wptr_[i] = *pos;\
			-- pos;\
		}\
	}\
	used_ += type_size;\
	wptr_ += type_size;\


#define DECODE(data, type)\
	int32_t type_size = sizeof(type);\
	if(used_ >= rsize_ + type_size)\
	{\
		uint8_t* pos = (uint8_t*)&data;\
		if(big_endian)\
		{\
			::memcpy(pos ,wptr_, type_size);\
		}\
		else\
		{\
			pos = pos + type_size - 1;\
			for(int32_t i = 0; i < type_size; ++i)\
			{\
				*pos = rptr_[i];\
				 -- pos; \
			}\
		}\
		rsize_ += type_size;\
		rptr_ += type_size;\
	}\
	else \
	{\
		memset(&data, 0x00, type_size);\
	}

template<class Elem, uint32_t CAPACITY>
class BinStream_T
{
public:
	typedef BinStream_T<Elem, CAPACITY> _MyBint;

	BinStream_T();
	virtual ~BinStream_T();

	//复位
	void		rewind(bool reset = false);
	void		resize(uint32_t new_size);
	//瘦身，只有在闲置状态下调用
	void		reduce();
	void		set_used_size(uint32_t used)
	{
		used_ = used;
	};

	const Elem* get_data_ptr() const
	{
		return data_;
	};

	uint8_t*	get_wptr()
	{
		return wptr_;
	};

	const uint8_t* get_rptr() const
	{
		return rptr_;
	};

	//获取缓冲区的大小
	uint32_t	size() const
	{
		return size_;
	};
	//获取数据填充区的大小
	uint32_t	data_size() const
	{
		return used_;
	};

	_MyBint& operator<<(bool val)
	{
		ENCODE(val, bool);
		return (*this);
	};

	_MyBint& operator>>(bool& val)
	{
		DECODE(val, bool);
		return (*this);
	};

	_MyBint& operator<<(int8_t val)
	{
		ENCODE(val, int8_t);
		return (*this);
	};

	_MyBint& operator>>(int8_t& val)
	{
		DECODE(val, int8_t);
		return (*this);
	};

	_MyBint& operator<<(int16_t val)
	{
		ENCODE(val, int16_t);
		return (*this);
	};

	_MyBint& operator>>(int16_t& val)
	{
		DECODE(val, int16_t);
		return (*this);
	};

	_MyBint& operator<<(int32_t val)
	{
		ENCODE(val, int32_t);
		return (*this);
	};
	_MyBint& operator>>(int32_t& val)
	{
		DECODE(val, int32_t);
		return (*this);
	};

	_MyBint& operator<<(int64_t val)
	{
		ENCODE(val, int64_t);
		return (*this);
	};

	_MyBint& operator>>(int64_t& val)
	{
		DECODE(val, int64_t);
		return (*this);
	};

	_MyBint& operator<<(uint8_t val)
	{
		ENCODE(val, uint8_t);
		return (*this);
	};

	_MyBint& operator>>(uint8_t& val)
	{
		DECODE(val, uint8_t);
		return (*this);
	};

	_MyBint& operator<<(uint16_t val)
	{
		ENCODE(val, uint16_t);
		return (*this);
	};

	_MyBint& operator>>(uint16_t& val)
	{
		DECODE(val, uint16_t);
		return (*this);
	};

	_MyBint& operator<<(uint32_t val)
	{
		ENCODE(val, uint32_t);
		return (*this);
	};

	_MyBint& operator>>(uint32_t& val)
	{
		DECODE(val, uint32_t);
		return (*this);
	};

	_MyBint& operator<<(uint64_t val)
	{
		ENCODE(val, uint64_t);
		return (*this);
	};

	_MyBint& operator>>(uint64_t& val)
	{
		DECODE(val, uint64_t);
		return (*this);
	};
	
	_MyBint& operator<<(const string& val)
	{
		uint32_t val_size = val.size();
		ENCODE(val_size, uint32_t);
	
		resize(used_ + val_size);
		::memcpy((void *)wptr_, (const void *)val.data(), (size_t)val_size);
		wptr_ += val_size;
		used_ += val_size;

		return (*this);
	};

	_MyBint& operator>>(string& val)
	{
		uint32_t val_size = 0;
		DECODE(val_size, uint32_t);

		if(val_size + rsize_ > used_) //防止越界访问
		{			
			throw 0;
		}
		else if(val_size == 0)
		{
			val = "";
		}
		else 
		{
			val.assign((char *)rptr_, val_size);

			rptr_ += val_size;
			rsize_ += val_size;
		}

		return (*this);
	};

	_MyBint& operator=(const _MyBint& strm)
	{
		resize(strm.size_);
		::memcpy(data_, strm.data_, strm.size_);
		used_ = strm.used_;
		rsize_ = strm.rsize_;
		rptr_ = data_ + rsize_;
		wptr_ = data_ + used_;

		return *this;
	}

	_MyBint& operator=(const string& data)
	{
		rewind(true);
		resize(data.size());
		set_used_size(data.size());

		::memcpy(get_wptr(), data.data(), data.size());
		wptr_ = data_ + used_;

		return *this;
	}

	void push_data(const uint8_t *data, uint32_t data_len)
	{
		ENCODE(data_len, uint32_t);

		resize(used_ + data_len);
		::memcpy((void *)wptr_, (const void *)data, (size_t)data_len);
		wptr_ += data_len;
		used_ += data_len;
	}

	void bin_to_string(string& data)
	{
		data.clear();
		data.assign((char*)rptr_, data_size());
	}

	const string to_string()
	{
		string text;
		return bin2asc((uint8_t *)data_, used_);
	}

protected:
	Elem*		data_;	//数据缓冲区
	uint8_t*	rptr_;	//当前读位置指针
	uint8_t*	wptr_;	//当前写位置指针

	size_t		size_;	//最大缓冲区尺寸
	size_t		used_;	//已经使用的缓冲区尺寸
	size_t		rsize_;	//读取的字节数

	bool		big_endian;//系统字节序标志
};

typedef BinStream_T<uint8_t, DEFAULT_PACKET_SIZE> BinStream;

BASE_NAMESPACE_END_DECL

#include "base_bin_stream.inl"

#endif

/************************************************************************************/



