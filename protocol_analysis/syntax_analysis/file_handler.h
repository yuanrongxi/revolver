#ifndef FILE_HANDLER_H_
#define FILE_HANDLER_H_

#include <string>
#include <fstream>

#include "base_block_handler.h"

using namespace std;

class CFileHandler
{
public:
	CFileHandler(string& str_fpath);
	virtual ~CFileHandler();

public:
	void	AnalysisFile();

	bool	IsOccurError(){return is_occur_error_;}

private:
	void	OpenFile(ifstream& infile);

	//分析一行中第一个单词
	string AnalysisFirstword(string& str_line);

	void	SetBlockHandler(string& str_keyword);

	//处理空行和#注释行
	bool	IsSpaceorConmentLine(string& str_line);

	bool	IsBlockBeginSign(string& str_line);

	//去掉行开始的空格和tab
	void	StripSpaceAndTab(string& str_line);

private:
	string	fname_path_;//def源文件的路径，相对路径或绝对路径
	int		fline_num_;

	bool	is_begin_block_;
	bool	is_occur_error_;//是否捕获了错误

	CBaseBlockHandler* handle_block_;
};
#endif //FILE_HANDLER_H_