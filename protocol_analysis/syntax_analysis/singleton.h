#ifndef SINGLETON_H_
#define SINGLETON_H_

template <class T>
class CSingleton
{
public:
	~CSingleton()
	{
	}
public:
	static T* CreateInstance()
	{
		return GetInstance();
	}
	static T* GetInstance()
	{
		if (p_T_ == NULL)
		{
			return p_T_ = new T;
		}
		else
		{
			return p_T_;
		}
	}
	static void DestoryInstance()
	{
		if (p_T_ != NULL)
		{
			delete p_T_;
			p_T_ = NULL;
		}
	}
private:
	CSingleton()
	{
	}
private:
	static T* p_T_;
};

template<class T>
T* CSingleton<T>::p_T_ = NULL;
#endif //SINGLETON_H_