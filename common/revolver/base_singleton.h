#ifndef __BASE_SIGNLETON_H
#define __BASE_SIGNLETON_H

#include <stdlib.h>

template<class T>
class CSingleton
{
public:
	static T* instance()
	{
		if(obj_ == NULL)
		{
			obj_ = new T();
		}

		return obj_;
	};

	static void destroy()
	{
		if(obj_ != NULL)
		{
			delete obj_;
			obj_ = NULL;
		}
	};

protected:
	CSingleton()
	{
	};

	virtual ~CSingleton()
	{
	};

protected:
	static T*	obj_;

private:
	CSingleton(const CSingleton&)
	{
	};

	CSingleton& operator=(const CSingleton&)
	{
	};
};


template <class T>
T* CSingleton<T>::obj_ = NULL;

#endif

