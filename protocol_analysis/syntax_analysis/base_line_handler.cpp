#include <cctype>
#include "base_line_handler.h"
#include "basedefine.h"

CBaseLineHandler::CBaseLineHandler()
{

}

CBaseLineHandler::~CBaseLineHandler()
{

}

void CBaseLineHandler::AnalysisLine(string& str_line)
{

}

void CBaseLineHandler::SplitLineWithSpace(string& str_line, StringArray& v_elem)
{
	StrSizeType elem_first_index = 0;
	StrSizeType elem_len = 0;
	for (StrSizeType i = 0; i < str_line.size(); i++)
	{
		if (str_line[i] != ' ' && str_line[i] != '\t')
		{
			if (elem_len == 0)
			{
				elem_first_index = i;
			}
			elem_len++;
		}
		else
		{
			if (elem_len != 0)
			{
				v_elem.push_back(str_line.substr(elem_first_index, elem_len));
				elem_first_index = 0;
				elem_len = 0;
			}

		}
	}
	//处理最后一个元素
	if (elem_len != 0)
	{
		v_elem.push_back(str_line.substr(elem_first_index, elem_len));
	}
}

bool CBaseLineHandler::IsVarNameLegal(string& str_varname)
{
	if (str_varname.empty())
	{
		return false;
	}
	if (!isalpha(str_varname[0]))
	{
		return false;
	}
	for (StrSizeType i = 1; i < str_varname.size(); i++)
	{
		if (isalpha(str_varname[i]))
		{
			continue;
		}
		else if (isdigit(str_varname[i]))
		{
			continue;
		}
		else if (str_varname[i] == '_')
		{
			continue;
		}
		else
		{
			return false;
		}
	}
	return true;
}

void CBaseLineHandler::SplitComplexType(const string& str_type, StringArray& v_type)
{
	StrSizeType first_split_index = str_type.find('<');
	StrSizeType second_split_index = str_type.find('>');

	if (first_split_index == string::npos || second_split_index == string::npos)
	{
		v_type.clear();
		return;
	}

	v_type.push_back(str_type.substr(0, first_split_index));
	v_type.push_back(str_type.substr(first_split_index + 1, second_split_index - first_split_index - 1));
}

string CBaseLineHandler::GenerateNewAlias(string str1, string str2)
{
	str1.replace(0, 1, 1, toupper(str1[0]));
	str2.replace(0, 1, 1, toupper(str2[0]));

	return str1 + "_" + str2;
}

string CBaseLineHandler::StripSpaceAndTab(string str)
{
	StrSizeType left_space = 0;
	StrSizeType right_space = 0;
	for (StrSizeType i = 0; i < str.size(); i++)
	{
		if (str[i] != ' ' && str[i] != '\t')
		{
			break;
		}
		else
		{
			left_space++;
		}
	}
	if (left_space == str.size())
	{
		return string();
	}
	for (StrSizeType i = str.size() - 1; i >= 0; i--)
	{
		if (str[i] != ' ' && str[i] != '\t')
		{
			break;
		}
		else
		{
			right_space++;
		}
	}	

	return str.substr(left_space, str.size() - left_space - right_space);
}

void CBaseLineHandler::StringToLower(string& str)
{
	for (StrSizeType i = 0; i < str.size(); i++)
	{
		if (str[i] >= 65 && str[i] <= 90)
		{
			str[i] += 32;
		}
	}
}

void CBaseLineHandler::StringToUpper(string& str)
{
	for (StrSizeType i = 0; i < str.size(); i++)
	{
		if (str[i] >= 97 && str[i] <= 122)
		{
			str[i] -= 32;
		}
	}
}

BaseType CBaseLineHandler::WhichType(const string& str)
{
	for (size_t i = 0; i < BASEDATATYPENUM; i++)
	{
		size_t index = str.find(g_basedatatype[i]);
		if (index != string::npos 
			&& strlen(g_basedatatype[i]) == str.size() 
			&& g_basedatatype[i][0] == str[0])
		{
			return (BaseType)i;
		}
	}
	return enm_type_none;
}

BaseType CBaseLineHandler::AnalysisValueType(const string& type)
{
	static int level = 0;//函数返回必须置0

	if (type.empty() || level > 3/*最多4层递归*/)
	{
		level = 0;
		return enm_type_none;
	}
	
	string src_type = GET_TYPEDEFSET()->FindSrcTypeFromAlias(type);
	if (src_type.empty())
	{
		src_type = type;
	}
	else
	{
		level++;
		return AnalysisValueType(src_type);
	}

	size_t pos = src_type.find("array");
	if (pos != string::npos)
	{
		level = 0;
		return enm_type_array;
	}

	BaseType t = CBaseLineHandler::WhichType(src_type);

	if (t != enm_type_none)
	{
		level = 0;
		return t;
	}

	if (GET_USERCLASSMAP()->IsUserDataType(src_type) 
		|| GET_MSGDEFMAP()->IsMsgType(src_type)
		|| GET_DATATYPESET()->IsContainType(src_type))
	{
		level = 0;
		return enm_type_obj;
	}
	return enm_type_none;
}

bool CBaseLineHandler::IsCanConvertNum(const string& str)
{
	if (str.size() <= 2 && isdigit(str[0]) != 0 && isdigit(str[1]) != 0)
	{
		return true;
	}
	if (str.substr(0, 2) == "0x" || str.substr(0, 2) == "0X")
	{
		for (size_t i = 2; i < str.size(); i++)
		{
			if (isdigit(str[i]) != 0)
			{
				continue;
			}

			if (str[i] < 65 || (70 < str[i] && str[i] < 97) || str[i] > 102)
			{
				return false;
			}
		}
	}
	else
	{
		for (size_t i = 0; i < str.size(); i++)
		{
			if (isdigit(str[i]) == 0)
			{
				return false;
			}
		}
	}
	return true;
}