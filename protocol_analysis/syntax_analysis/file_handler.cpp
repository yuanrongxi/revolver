#include "file_handler.h"
#include "basedefine.h"
#include "base_error.h"

#include <ctype.h>
#include <iostream>
#include <assert.h>

CFileHandler::CFileHandler(string& str_fpath):fname_path_(str_fpath)
{
	handle_block_ = NULL;

	is_begin_block_ = false;
	is_occur_error_ = false;

	fline_num_ = 0;
}

CFileHandler::~CFileHandler()
{
	DESTORY_BLOCKHANDLERMAP();
	handle_block_ = NULL;
}
void CFileHandler::SetBlockHandler(string& str_keyword)
{
	CBaseBlockHandler* p_blockhandler = GET_BLOCKHANDLERMAP()->FetchKeyWordHandler(str_keyword);
	
	if (p_blockhandler == NULL)
	{
		//to do:´íÎó¹Ø¼ü×Ö
		THROW_ERROR(fline_num_, str_keyword + " not defined");
	}

	handle_block_ = p_blockhandler;

// 	cout << str_keyword << endl;
}
void CFileHandler::AnalysisFile()
{
	ifstream infile;
	OpenFile(infile);

	while (!infile.eof())
	{
		fline_num_++;

		string str_line;
		getline(infile, str_line);

		StripSpaceAndTab(str_line);

		if (IsSpaceorConmentLine(str_line))
		{
			continue;
		}

		string str_keyword = AnalysisFirstword(str_line);

		try
		{		
			if(!IsBlockBeginSign(str_line) && !str_keyword.empty())
			{
				SetBlockHandler(str_keyword);
			}
			if (handle_block_ != NULL)
			{
				handle_block_->AnalysisBlockContent(str_line, fline_num_);
			}
		}
		catch(const CBaseError& e)
		{
			CBaseErrorHandler errorhandler;
			errorhandler.ErrorHandle(e);

			if (!is_occur_error_)
			{
				is_occur_error_ = true;
			}
		}
	}
}

void CFileHandler::OpenFile(ifstream& infile)
{
	infile.open(fname_path_.c_str(), ifstream::in);
	if(!infile)
	{
		cerr << "error: unable to open input file:" << fname_path_ << endl;
		exit(0);
	}
}

string CFileHandler::AnalysisFirstword(string& str_line)
{
	StrSizeType first_letter_index = string::npos;
	for(StrSizeType i = 0; i < str_line.size(); i++)
	{
		if(isalpha(str_line[i]))
		{
			first_letter_index = i;
			break;
		}
	}
	StrSizeType last_letter_index = first_letter_index;
	for(StrSizeType i = first_letter_index; i < str_line.size(); i++)
	{
		if(!isalpha(str_line[i]))
		{
			last_letter_index = i - 1;
			break;
		}
	}
	// cout << first_letter_index << last_letter_index << endl;
	if (first_letter_index == string::npos)
	{
		return string();
	}
	return str_line.substr(first_letter_index, last_letter_index - first_letter_index + 1);
}

bool CFileHandler::IsSpaceorConmentLine(string& str_line)
{
	for (StrSizeType i = 0; i < str_line.size(); i++)
	{
		if (str_line[i] == '#')
		{
			return true;
		}
		if (str_line[i] != ' ' && str_line[i] != '\t')
		{
			return false;
		}
	}
	return true;
}

bool CFileHandler::IsBlockBeginSign(string& str_line)
{
	if(str_line.find('{') != string::npos)
	{
// 		cout << "IsBlockBeginSign:{" << endl;
		is_begin_block_ = true;
	}
// 	assert(str_line.find('}') == string::npos);

	if(str_line.find('}') != string::npos)
	{
// 		cout << "IsBlockBeginSign:} error" << endl;
		is_begin_block_ = false;
	}

	return is_begin_block_;
}

void CFileHandler::StripSpaceAndTab(string& str_line)
{
	StrSizeType left_space = 0;
	StrSizeType right_space = 0;

	for (StrSizeType i = 0; i < str_line.size(); i++)
	{
		if (str_line[i] != ' ' && str_line[i] != '\t')
		{
			break;
		}
		else
		{
			left_space++;
		}
	}

	if (left_space == str_line.size())
	{
		str_line.clear();
		return;
	}

	for (StrSizeType i = str_line.size() - 1; i >= 0; i--)
	{
		if (str_line[i] != ' ' && str_line[i] != '\t')
		{
			break;
		}
		else
		{
			right_space++;
		}
	}	
	
	string sub_str = str_line.substr(left_space, str_line.size() - left_space - right_space);
	str_line.replace(0, str_line.size(), sub_str);
}