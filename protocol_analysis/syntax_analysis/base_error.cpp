#include "base_error.h"
#include <iostream>

CBaseError::CBaseError(int line_num /* = 0 */, string str /* = ""*/ ) : error_line_num_(line_num),
				error_desc_(str)
{

}

CBaseError::~CBaseError()
{

}

CBaseErrorHandler::CBaseErrorHandler()
{

}

CBaseErrorHandler::~CBaseErrorHandler()
{

}

void CBaseErrorHandler::ErrorHandle(const CBaseError& error)
{
	std::cerr << "line:" << error.error_line_num_ << " " << error.error_desc_ << std::endl;
}