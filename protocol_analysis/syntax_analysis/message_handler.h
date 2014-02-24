#ifndef MESSAGE_HANDLER_H_
#define MESSAGE_HANDLER_H_

#include "base_block_handler.h"
#include "basedefine.h"

class CMessageDefHandler : public CBaseBlockHandler
{
public:
	CMessageDefHandler();
	virtual ~CMessageDefHandler();

public:
	void AnalysisBlockContent(string& str_line, int line_num);

private:
	void CheckMessageDef(string& str_line, int line_num);

private:
	string		msgid_;
	string		msgname_;
	MemItemVec	typedesc_;
};
#endif //MESSAGE_HANDLER_H_