#include "file_handler.h"
#include "base_line_handler.h"
#include "basedefine.h"
#include "namespace_handler.h"
#include "base_error.h"
#include <iostream>

CNameSpaceHandler::CNameSpaceHandler()
{
	//设置处理关键字keyword的块
	if (!GET_KEYWORDSET()->IsKeyWord(string(NAME_SPACE)))
	{
		std::cerr << "CNameSpaceHandler:bind wrong key word" << std::endl;
		abort();
	}
	str_keyword_ = string(NAME_SPACE);
}

CNameSpaceHandler::~CNameSpaceHandler()
{

}

void CNameSpaceHandler::AnalysisBlockContent(string& str_line, int line_num)
{
	StringArray v_elem;
	CBaseLineHandler::SplitLineWithSpace(str_line, v_elem);
	if (v_elem.size() != 3)
	{
		//to do:格式不正确
		THROW_ERROR(line_num, "Error:wrong form, expected 'namespace = sample'");
	}
	if (!(GET_KEYWORDSET()->IsKeyWord(v_elem[0])))
	{
		//to do:关键字不正确
		THROW_ERROR(line_num, "Error:wrong key word, expected 'namespace'");
	}
	if (v_elem[1] != "=")
	{
		//to do:bad syntax
		THROW_ERROR(line_num, "Error:bad syntax, expected '='");
	}
	if (!CBaseLineHandler::IsVarNameLegal(v_elem[2]))
	{
		//to do:名字不合法
		THROW_ERROR(line_num, string("Warning:") + v_elem[2] + " is a bad name, need to change");
	}
	//to do:将v_elem[2]加入到文件描述表
	GET_FILEDESCMAP()->AddFileDesc(v_elem[0], v_elem[2]);
}