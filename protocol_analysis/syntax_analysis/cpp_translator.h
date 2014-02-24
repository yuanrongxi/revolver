#ifndef CPP_TRANSLATOR_H_
#define CPP_TRANSLATOR_H_

#include "base_translator.h"
#include "basedefine.h"
#include <fstream>

#define COMPTABLEROW  2
#define COMPTABLECOLM 9

using namespace std;

typedef map<string, string> TypeTransTable;

class CCppTranslator : public CBaseTranslator
{
public:
	CCppTranslator();
	~CCppTranslator();

public:
	void	TranslateProtocol();

private:
	void	OpenObjFile();
	void	CloseObjFile();

	void	AddFileDescToFile();
	void	AddVarDefToFile();
	void	AddTypeDefToFile();
	void	AddUserClassDef();
	void	AddMsgBodyDef();

	void	FillConstructorAndDataMem(const string& str_name, const MemItemVec& mem_set);
	void	FillEncodeFunction(const MemItemVec& mem_set);
	void	FillDecodeFunction(const MemItemVec& mem_set);
	void	OutputArrayToFile(const string& str_name, const string& str_op, const string& strm_name = "strm");
	void	OutputLeftFileDesc();
	void	OutputLeftTypedef();
	void	OutputInetAddrType(const string& str_vname, const string& str_op);

	inline void	Output8to16Convert(const string& type);

	void    GenerateBinStreamFunction(const string& str_type, const MemItemVec& v_mem, const string& str_op);
	void	GeneratePrintFunction(const string& str_type, const MemItemVec& v_mem);
	void	GenerateOstreamFunction(const string& str_type, const MemItemVec& v_mem);
	

private:
	ofstream		out_file_;
	StringArray		left_file_desc_;//一个保存前文对应的输出信息的栈
	StringArray		left_typedef_;
};

class CTypeTransTable
{
public:
	CTypeTransTable();
	~CTypeTransTable();

public:
	string	GetCorrespondType(const string& str_type);

private:
	TypeTransTable typetrans_table_;
};

ofstream& operator <<(ofstream& fout, const CMsgDefDesc &msgdesc);
ofstream& operator <<(ofstream& fout, const CTypedefDesc& typedefdesc);
ofstream& operator <<(ofstream& fout, const CMemItem& item);

#define CREATE_TRANSTABLE		CSingleton<CTypeTransTable>::CreateInstance
#define GET_TRANSTABLE			CSingleton<CTypeTransTable>::GetInstance
#define DESTORY_TRANSTABLE		CSingleton<CTypeTransTable>::DestoryInstance

#endif