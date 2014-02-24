#ifndef BASE_BLOCK_HANDLER_H_
#define BASE_BLOCK_HANDLER_H_

#include "basedefine.h"
#include <string>

using namespace std;

class CBaseBlockHandler
{
public:
	CBaseBlockHandler();
	virtual ~CBaseBlockHandler();

public:
	virtual void	AnalysisBlockContent(string& str_line, int line_num);
	string			GetKeyWord(){return str_keyword_;}

protected:
	string	str_keyword_;
};
#endif //BASE_BLOCK_HANDLER_H_