#ifndef BASE_ERROR_H_
#define BASE_ERROR_H_

#include <string>

using namespace std;

class CBaseError
{
public:
	CBaseError(int line_num = 0, string str = "");
	virtual ~CBaseError();

public:
	int		error_line_num_;//第几行出错
	string	error_desc_;
};

class CBaseErrorHandler
{
public:
	CBaseErrorHandler();
	virtual ~CBaseErrorHandler();

public:
	virtual void ErrorHandle(const CBaseError& error);
};
#endif //BASE_ERROR_H_