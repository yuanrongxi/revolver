#ifndef TYPE_DEFINE_HANDLER_H_
#define TYPE_DEFINE_HANDLER_H_

#include "base_block_handler.h"

class CTypedefHandler : public CBaseBlockHandler
{
public:
	CTypedefHandler();
	virtual ~CTypedefHandler();

	void AnalysisBlockContent(string& str_line, int line_num);
private:
	void CheckComplexType(string& str_type, int line_num);
};
#endif //TYPE_DEFINE_HANDLER_H_