#ifndef FILE_TITLE_HANDLER_H_
#define FILE_TITLE_HANDLER_H_

#include "base_block_handler.h"
class CFileTitleHandler : public CBaseBlockHandler
{
public:
	CFileTitleHandler();
	virtual ~CFileTitleHandler();

public:
	void AnalysisBlockContent(string& str_line, int line_num);
};
#endif //FILE_TITLE_HANDLER_H_