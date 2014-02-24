#include "file_handler.h"
#include "base_line_handler.h"
#include "basedefine.h"
#include "typedef_handler.h"
#include "base_error.h"
#include <iostream>

CTypedefHandler::CTypedefHandler()
{
	if (!GET_KEYWORDSET()->IsKeyWord(string(TYPE_DEFINE)))
	{
		std::cerr << "CTypedefHandler:bind wrong key word" << std::endl;
		abort();
	}
	str_keyword_ = string(TYPE_DEFINE);
}

CTypedefHandler::~CTypedefHandler()
{

}


void CTypedefHandler::AnalysisBlockContent(string& str_line, int line_num)
{
// 	cout << "CTypedefHandler::AnalysisBlockContent " << line_num << endl;
	StringArray v_elem;
	CBaseLineHandler::SplitLineWithSpace(str_line, v_elem);

	if (v_elem.size() != 4)
	{
		//to do:格式不正确
		THROW_ERROR(line_num, "Error:wrong form, expected 'typedef type<type> : OTHER_NAME'");
	}

	if (!(GET_KEYWORDSET()->IsKeyWord(v_elem[0])))
	{
		//to do:关键字不正确
		THROW_ERROR(line_num, "Error:wrong key word, expected 'typedef'");
	}

	//检查可能的复合类型
	CheckComplexType(v_elem[1], line_num);

	if (v_elem[2] != ":")
	{
		//to do:bad syntax
		THROW_ERROR(line_num, "Error:bad syntax, expected ':'");
	}

	if (GET_DATATYPESET()->IsContainType(v_elem[3]))
	{
		THROW_ERROR(line_num, string("Error:type ") + v_elem[3] + " Duplicate Defined");
	}

	if (!CBaseLineHandler::IsVarNameLegal(v_elem[3]))
	{
		//to do:名字不合法
		THROW_ERROR(line_num, string("Warning:") + v_elem[3] + " is a bad name, need to change");
	}

	GET_DATATYPESET()->AddDataType(v_elem[3]);
	//to do:加入typedef描述表
	GET_TYPEDEFSET()->AddTypedef(v_elem[1], v_elem[3]);
}

void CTypedefHandler::CheckComplexType(string& str_type, int line_num)
{
	StringArray v_type;
	CBaseLineHandler::SplitComplexType(str_type, v_type);
	if (v_type.empty())
	{
		if (!(GET_DATATYPESET()->IsContainType(str_type)))
		{
			//to do:关键字不正确
			THROW_ERROR(line_num, string("Error:type ") + str_type + " not defined");
		}
	}
	else
	{
		if (v_type.size() != 2)
		{
			THROW_ERROR(line_num, string("Error:type ") + str_type + " wrong define");
		}

		for (vector<string>::size_type i = 0; i < v_type.size(); i++)
		{
			if (!(GET_DATATYPESET()->IsContainType(v_type[i])))
			{
				//to do:关键字不正确
				THROW_ERROR(line_num, string("Error:type ") + v_type[i] + " not defined");
			}
		}

		if (v_type[1] == "array")
		{
			THROW_ERROR(line_num, string("Error:you need specify a base type"));
		}
	}
}