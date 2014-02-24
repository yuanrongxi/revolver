/*************************************************************************************
*filename:	base_cache_buffer.h
*
*to do:		实现一个缓冲单元的缓冲区，为了可以进行对应的查找和更新、替换操作，主要用户
			做媒体报文的缓冲区
*Create on: 2013-03
			2013-03 实现基本的缓冲区数组
*Author:	zerok
*check list:
*************************************************************************************/

#ifndef __BASE_CACHE_BUFFER_H_
#define __BASE_CACHE_BUFFER_H_

#include "base_typedef.h"
#include "base_namespace.h"

BASE_NAMESPACE_BEGIN_DECL

//T类型一定要是指针类型!!!!
template<typename T, int32_t CAPACITY>
class CacheBuffer_T
{
public:
	CacheBuffer_T()
	{
		min_index_ = 0;
		max_index_ = 0;

		min_key_ = 0;
		max_key_ = 0;

		buffer_ = new T[CAPACITY];
	};

	virtual ~CacheBuffer_T()
	{
		if(buffer_ != NULL)
		{
			delete []buffer_;
		}

		min_index_ = 0;
		max_index_ = 0;

		min_key_ = 0;
		max_key_ = 0;
	};

	T find(uint32_t key)
	{
		T ret = NULL;
		if(key >= min_key_ && key <= max_key_)
		{
			uint32_t index = (key - min_key_ + min_index_) % CAPACITY;
			ret = buffer_[index];
		}

		return ret;
	}

	//插入一个单元
	bool insert(uint32_t key, const T& data)
	{
		bool ret = false;

		if(key > min_key_ && key < max_key_)
		{
			uint32_t index = (key - min_key_ + min_index_) % CAPACITY;
			if(buffer_[index] == NULL)
			{
				buffer_[index] = data;
				ret = true;
			}
		}
		else if(key > max_key_)
		{ 
			uint32_t key_space = key - max_key_;
			for(uint32_t i = 0; i < key_space; ++i)
			{
				max_index_ ++;
				if(max_index_ == min_index_) //往前移动
				{
					min_index_ ++;
					min_index_ = min_index_ % CAPACITY;
				}

				max_index_ = max_index_ % CAPACITY;
				buffer_[max_index_] = NULL;
			}

			buffer_[max_index_] = data;

			max_key_ = key;
			if(min_key_ == 0)
			{
				min_key_ = key;
			}

			ret = true;
		}

		return ret;
	}

	//删除最前面的单元
	T erase()
	{
		T ret = NULL;
		if(emtpy())
			return ret;

		if(max_index_ == min_index_)
		{
			ret = buffer_[min_index_];
			buffer_[min_index_] = NULL;

			min_index_ = 0;
			max_index_ = 0;	
		}
		else
		{
			ret = buffer_[min_index_];
			buffer_[min_index_] = NULL;

			min_index_ ++;
			min_index_ = min_index_ % CAPACITY;

			min_key_ ++;
		}

		return ret;
	}

	bool emtpy() const
	{
		return ((max_index_ == 0 && min_index_ == 0 && buffer_[min_index_] == NULL) ? true : false);
	};


private:
	T*			buffer_;

	uint32_t	min_index_;		//最小序号的位置
	uint32_t	max_index_;		//最大序号的位置

	uint32_t	min_key_;		//最小序号
	uint32_t	max_key_;		//最大序号
};

BASE_NAMESPACE_END_DECL

#endif


/************************************************************************************/


