#include "file_handler.h"
#include "base_line_handler.h"
#include "basedefine.h"
#include "datatype_handler.h"
#include "base_error.h"
#include "memdesc_handler.h"
#include <iostream>

CDatatypeHandler::CDatatypeHandler()
{
	if (!GET_KEYWORDSET()->IsKeyWord(string(DATA_TYPE)))
	{
		std::cerr << "CDatatypeHandler:bind wrong key word" << std::endl;
		abort();
	}
	str_keyword_ = string(DATA_TYPE);
}

CDatatypeHandler::~CDatatypeHandler()
{

}

void CDatatypeHandler::AnalysisBlockContent(string& str_line, int line_num)
{
	if (str_line.find('(') != string::npos)
	{
// 		cout << "CDatatypeHandler::AnalysisBlockContent " << line_num << endl;
		CheckTypeDef(str_line, line_num);
		return;
	}

	if (str_line.find('{') != string::npos)
	{
		return;
	}

	if (str_line.find('}') != string::npos)
	{
		GET_USERCLASSMAP()->AddUserClass(type_name_, memdesc_);
		type_name_.clear();
		memdesc_.clear();
		return;
	}

	StringArray v_elem;
	CBaseLineHandler::SplitLineWithSpace(str_line, v_elem);
	if (v_elem[0] == type_name_)
	{
		THROW_ERROR(line_num, "Error: can not nested definition");
	}

	CMemItem item = CMemDescHandler::CheckMemDesc(str_line, line_num);
	if (CMemDescHandler::IsMemNameNotUsed(item.m_name, memdesc_, line_num))
	{
		memdesc_.push_back(item);
	}
}

void CDatatypeHandler::CheckTypeDef(string& str_line, int line_num)
{
	StrSizeType lbrackets_index = str_line.find('(');
	StrSizeType rbrackets_index = str_line.find(')');

	if (lbrackets_index != string::npos)
	{
		if (rbrackets_index == string::npos)
		{
			THROW_ERROR(line_num, "Warning:bad syntax, expected a ')'");
		}

		string str_kword = str_line.substr(0, lbrackets_index);
		if (!(GET_KEYWORDSET()->IsKeyWord(str_kword)))
		{
			THROW_ERROR(line_num, "Error:wrong key word, expected 'datatype'");
		}

		string str_type = str_line.substr(lbrackets_index + 1, rbrackets_index - lbrackets_index - 1);
		if (GET_DATATYPESET()->IsContainType(str_type))
		{
			THROW_ERROR(line_num, string("Error:type ") + str_type + " Duplicate Defined");
		}

		if (!CBaseLineHandler::IsVarNameLegal(str_type))
		{
			THROW_ERROR(line_num, string("Warning:") + str_type + " is bad name, need to change");
		}

		type_name_ = str_type;
		GET_DATATYPESET()->AddDataType(type_name_);
	}
}
