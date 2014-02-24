#include "var_handler.h"
#include "file_handler.h"
#include "base_line_handler.h"
#include "basedefine.h"
#include "base_error.h"
#include <iostream>

CVarHandler::CVarHandler()
{
	//设置处理关键字var的块
	if (!GET_KEYWORDSET()->IsKeyWord(string(VAR)))
	{
		std::cerr << "CVarHandler:bind wrong key word" << std::endl;
		abort();
	}
	str_keyword_ = string(VAR);
}

CVarHandler::~CVarHandler()
{

}

void CVarHandler::AnalysisBlockContent(string& str_line, int line_num)
{
	StringArray v_elem;
	CBaseLineHandler::SplitLineWithSpace(str_line, v_elem);

// 	cout << v_elem.size() << v_elem[0] << endl;
	if (v_elem.size() != 4)
	{
		THROW_ERROR(line_num, "Error:wrong form, expected 'var var_name = value'");
	}

	if (!(GET_KEYWORDSET()->IsKeyWord(v_elem[0])))
	{
		THROW_ERROR(line_num, "Error:wrong key word, expected 'var'");
	}

	if (!CBaseLineHandler::IsVarNameLegal(v_elem[1]))
	{
		THROW_ERROR(line_num, string("Warning:") + v_elem[1] + " is a bad name, need to change");
	}

	if (v_elem[2] != "=")
	{
		//to do:bad syntax
		THROW_ERROR(line_num, "Error:bad syntax, expected '='");
	}

	//to do:将变量名和值加入变量描述表
	GET_VARDEFMAP()->AddVarDef(v_elem[1], v_elem[3]);
}