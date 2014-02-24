#ifndef VAR_HANDLER_H_
#define VAR_HANDLER_H_

#include "base_block_handler.h"

class CVarHandler : public CBaseBlockHandler
{
public:
	CVarHandler();
	virtual ~CVarHandler();

public:
	void AnalysisBlockContent(string& str_line, int line_num);
};
#endif //VAR_HANDLER_H_