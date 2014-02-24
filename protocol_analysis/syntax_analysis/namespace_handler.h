#ifndef NAMESPACE_HANDLER_H_
#define NAMESPACE_HANDLER_H_

#include "base_block_handler.h"

class CNameSpaceHandler : public CBaseBlockHandler
{
public:
	CNameSpaceHandler();
	virtual ~CNameSpaceHandler();

public:
	void AnalysisBlockContent(string& str_line, int line_num);
};
#endif //NAMESPACE_HANDLER_H_