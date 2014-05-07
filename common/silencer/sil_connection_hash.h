/*************************************************************************************
*filename:	sil_connection_hash.h
*
*to do:		定义一个连接对象的HASH管理对象,主要用于前端服务器管理多客户端连接用
管理
*Create on: 2013-07
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __SIL_CONNECTION_HASH_H
#define __SIL_CONNECTION_HASH_H

#include "revolver/base_typedef.h"
#include "revolver/base_namespace.h"
#include "revolver/base_os.h"

#include <list>

using namespace std;

typedef list<uint32_t>	FreeHashIDS;

BASE_NAMESPACE_BEGIN_DECL

#define CONN_HASH_SIZE	256
#define INVALID_CONN_ID 0xFFFFFFFF

template<class T>
class SilConnHash_T
{
public:
	SilConnHash_T()
	{
		conn_hash_ = (T**)malloc(sizeof(T*) * CONN_HASH_SIZE);
		hash_size_ = CONN_HASH_SIZE;
		for(uint32_t i = 0; i < hash_size_; ++ i)
		{
			conn_hash_[i] = NULL;
			free_ids_.push_back(i);
		}
	};

	virtual ~SilConnHash_T()
	{
		if(conn_hash_ != NULL)
		{
			free(conn_hash_);
			conn_hash_ = NULL;
		}

		hash_size_ = 0;
		free_ids_.clear();
	};

	uint32_t add_conn(T* conn)
	{
		if(conn == NULL || hash_size_ >= CONN_HASH_SIZE * 1024)
		{
			return INVALID_CONN_ID;
		}

		if(free_ids_.empty())
		{
			resize();
		}

		uint32_t index = free_ids_.front();
		conn_hash_[index] = conn;
		free_ids_.pop_front();

		return index;
	};

	void del_conn(uint32_t index)
	{
		if(index >= 0 && index < hash_size_ && conn_hash_[index] != NULL)
		{
			conn_hash_[index] = NULL;
			free_ids_.push_back(index);
		}
	};

	T* find_conn(uint32_t index)
	{
		T* ret = NULL;
		if(index >= 0 && index < hash_size_)
		{
			ret = conn_hash_[index];
		}

		return ret;
	};

protected:
	void resize()
	{
		uint32_t re_size = hash_size_ + CONN_HASH_SIZE;
		conn_hash_ = (T**)realloc(conn_hash_, sizeof(T*) * re_size);
		for(uint32_t i = hash_size_; i <re_size; i ++)
		{
			conn_hash_[i] = NULL;
			free_ids_.push_back(i);
		}

		hash_size_ = re_size;
	};

protected:
	T**			conn_hash_;
	uint32_t	hash_size_;

	FreeHashIDS free_ids_;
};

BASE_NAMESPACE_END_DECL

#endif

/************************************************************************************/

