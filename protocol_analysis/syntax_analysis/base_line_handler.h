#ifndef BASE_LINE_HANDLER_H_
#define BASE_LINE_HANDLER_H_

#include "basedefine.h"
#include <string>
#include "global.h"

using namespace std;

class CBaseLineHandler
{
public:
	CBaseLineHandler();
	virtual ~CBaseLineHandler();

public:
	virtual void	AnalysisLine(string& str_line);

	//将行以空格为字符分割，取出包括单词、','、括号、冒号、变量名等元素
	//输入行不是注释或空行，v_elem接受分割后的元素
	static void		SplitLineWithSpace(string& str_line, StringArray& v_elem);

	//变量只包含字母数字下划线，并且以字母开头
	static bool		IsVarNameLegal(string& str_varname);

	//如果str_type是复合类型，如array<>形式的为复合类型,拆分复合类型成2个独立类型通过v_type返回
	//否则清空vector
	static void		SplitComplexType(const string& str_type, StringArray& v_type);

	//将2个字符串首字母转换成大写拼接生产一个新的字符串别名,如Array_Int8
	static string	GenerateNewAlias(string str1, string str2);

	//去掉字符串两端的空白
	static string	StripSpaceAndTab(string str);

	//将字符串所有的字母转换为小写
	static void		StringToLower(string& str);

	//将字符串所有的字母转换为大写
	static void		StringToUpper(string& str);

	//判断是哪种基本类型
	static BaseType WhichType(const string& str);

	static BaseType AnalysisValueType(const string& type);
	
	static bool		IsCanConvertNum(const string& str);
};
#endif //BASE_LINE_HANDLER_H_