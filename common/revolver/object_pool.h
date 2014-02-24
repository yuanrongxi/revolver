/*************************************************************************************
*filename:	object_pool.h
*
*to do:		定义对象池
*Create on: 2012-04
*Author:	zerok
*check list:
*************************************************************************************/
#ifndef __OBJECT_POOL_H
#define __OBJECT_POOL_H

#include "base_typedef.h"
#include "base_namespace.h"
#include "base_guard.h"

#include <list>

//#define MEM_MALLOC

BASE_NAMESPACE_BEGIN_DECL


template<typename T, int CAPACITY>
class ObjectPool
{
public:
	ObjectPool(){init();};
	~ObjectPool(){destroy();};

	void init()
	{
		obj_number_ = CAPACITY;
		pop_number_ = 0;

		T* obj = NULL;
		for(uint32_t i = 0; i < CAPACITY; i ++)
		{
			obj = new T;
			obj_list_.push_back(obj);
		}
	};

	void destroy()
	{
		T* obj = NULL;
		while(!obj_list_.empty())
		{
			obj = obj_list_.front();
			if(obj != NULL)
			{
				delete obj;
				obj = NULL;
			}

			obj_list_.pop_front();
		}

		obj_number_ = 0;
		pop_number_ = 0;
	}

	void alloc_object(uint32_t number)
	{
		T* obj = NULL;
		for(uint32_t i = 0; i < number; i ++)
		{
			obj = new T;
			obj_list_.push_back(obj);
		}

		obj_number_ += number;
	}

	void push_obj(T* obj)
	{
		if(obj == NULL)
			return ;

#ifndef MEM_MALLOC

		obj_list_.push_back(obj);

		if(pop_number_ > 0)
			-- pop_number_;
#else
		delete obj;
#endif
	}

	T* pop_obj()
	{
#ifndef MEM_MALLOC
		if(obj_list_.empty())
		{
			alloc_object(CAPACITY / 2 + 1);
		}

		T* obj = obj_list_.front();
		obj_list_.pop_front();

		pop_number_ ++;

		return obj;
#else
		T* obj = new T;

		return obj;
#endif
	}

	uint32_t size() 
	{
		return obj_list_.size();
	};
private:
	std::list<T*>	obj_list_;

public:
	uint32_t		obj_number_;
	uint32_t		pop_number_;
};


template<typename T, class BASE_LOCK, int CAPACITY>
class ObjectMutexPool
{
public:
	ObjectMutexPool(){init();};
	~ObjectMutexPool(){destroy();};

	void init()
	{
		BASE_GUARD(BASE_LOCK, cf_mon, lock_);

		obj_list_.init();
	};

	void destroy()
	{
		BASE_GUARD(BASE_LOCK, cf_mon, lock_);

		obj_list_.destroy();
	}

	void push_obj(T* obj)
	{
		if(obj == NULL)
			return ;

		BASE_GUARD(BASE_LOCK, cf_mon, lock_);

		obj_list_.push_obj(obj);
	}

	T* pop_obj()
	{
		BASE_GUARD_RETURN(BASE_LOCK, cf_mon, lock_, NULL);

		return obj_list_.pop_obj();
	}

	uint32_t size() 
	{
		BASE_GUARD_RETURN(BASE_LOCK, cf_mon, lock_, 0);

		return obj_list_.size();
	};

	uint32_t get_obj_number()
	{
		return obj_list_.obj_number_;
	};

	uint32_t get_pop_number()
	{
		return obj_list_.pop_number_;
	};

private:
	ObjectPool<T, CAPACITY>	obj_list_;
	BASE_LOCK				lock_;
};

BASE_NAMESPACE_END_DECL

#endif

/************************************************************************************/
