#include "file_handler.h"
#include "base_line_handler.h"
#include "basedefine.h"
#include "message_handler.h"
#include "base_error.h"
#include "memdesc_handler.h"
#include <iostream>

CMessageDefHandler::CMessageDefHandler()
{
	if (!GET_KEYWORDSET()->IsKeyWord(string(MESSAGE)))
	{
		std::cerr << "CMessageHandler:bind wrong key word" << std::endl;
		abort();
	}
	str_keyword_ = string(MESSAGE);
}

CMessageDefHandler::~CMessageDefHandler()
{

}

void CMessageDefHandler::AnalysisBlockContent(string& str_line, int line_num)
{
// 	cout << "line27 CMessageDefHandler:msgid_" << msgid_ << "msgname_" << msgname_ << endl;

	if (str_line.find('(') != string::npos)
	{
		CheckMessageDef(str_line, line_num);
		return;
	}

	if (str_line.find('{') != string::npos)
	{
		return;
	}

	if (str_line.find('}') != string::npos)
	{
		GET_DATATYPESET()->AddDataType(msgname_);
		CMsgDefDesc msgdef_desc(msgid_, msgname_);

// 		cout << "line44 CMessageDefHandler:msgid_" << msgid_ << "msgname_" << msgname_ << endl;

		GET_MSGDEFMAP()->AddMsgDef(msgdef_desc, typedesc_);
		msgid_.clear();
		msgname_.clear();
		typedesc_.clear();

// 		GET_MSGDEFMAP()->PrintSelfMap();
		return;
	}

	StringArray v_elem;
	CBaseLineHandler::SplitLineWithSpace(str_line, v_elem);
	if (v_elem[0] == msgname_)
	{
		THROW_ERROR(line_num, "Error: can not nested definition");
	}

	CMemItem item = CMemDescHandler::CheckMemDesc(str_line, line_num);
	if (CMemDescHandler::IsMemNameNotUsed(item.m_name, typedesc_, line_num))
	{
		typedesc_.push_back(item);
	}

// 	cout << "line67 CMessageDefHandler:msgid_" << msgid_ << "msgname_" << msgname_ << endl;
}

void CMessageDefHandler::CheckMessageDef(string& str_line, int line_num)
{
	StrSizeType lbracket_index = str_line.find('(');
	StrSizeType rbracket_index = str_line.find(')');
	StrSizeType comma_index = str_line.find(',');

	if (rbracket_index == string::npos || comma_index == string::npos)
	{
		THROW_ERROR(line_num, "Error:wrong form, expected 'message(CName,Msgid)'");
	}

	string str_temp = str_line.substr(0, lbracket_index);
	string str_kword = CBaseLineHandler::StripSpaceAndTab(str_temp);

	str_temp = str_line.substr(lbracket_index + 1, comma_index - lbracket_index - 1);
	string str_msgname = CBaseLineHandler::StripSpaceAndTab(str_temp);

	str_temp = str_line.substr(comma_index + 1, rbracket_index - comma_index - 1);
	string str_msgid = CBaseLineHandler::StripSpaceAndTab(str_temp);

	if (!(GET_KEYWORDSET()->IsKeyWord(str_kword)))
	{
		THROW_ERROR(line_num, "Error: key word error, need key word 'message'");
	}

	if (GET_DATATYPESET()->IsContainType(str_msgname))
	{
		THROW_ERROR(line_num, string("Error:message name ") + str_msgname + " Duplicate Defined");
	}

	if (GET_VARDEFMAP()->IsVarDefined(str_msgname))
	{
		THROW_ERROR(line_num, string("Error:message name ") + str_msgname + " Duplicate Defined as a var");
	}

	if (!GET_VARDEFMAP()->IsVarDefined(str_msgid))
	{
		THROW_ERROR(line_num, "Error:message id not defined");
	}

	string id_value = GET_VARDEFMAP()->fetch_value(str_msgid);
	if (!CBaseLineHandler::IsCanConvertNum(id_value))
	{
		THROW_ERROR(line_num, string("Error:message id ") + str_msgid + " is not a number");
	}
	if (GET_MSGDEFMAP()->IsMsgIDUsed(str_msgid))
	{
		THROW_ERROR(line_num, "Warning: message id reused");
	}
	msgname_ = str_msgname;
	msgid_ = str_msgid;
}
