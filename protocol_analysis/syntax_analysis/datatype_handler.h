#ifndef DATATYPE_HANDLER_H_
#define DATATYPE_HANDLER_H_

#include "base_block_handler.h"
#include "basedefine.h"

class CDatatypeHandler : public CBaseBlockHandler
{
public:
	CDatatypeHandler();
	virtual ~CDatatypeHandler();

public:
	void AnalysisBlockContent(string& str_line, int line_num);

private:
	void CheckTypeDef(string& str_line, int line_num);

private:
	string		type_name_;
	MemItemVec	memdesc_;
};
#endif //DATATYPE_HANDLER_H_