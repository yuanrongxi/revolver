#ifndef LUA_TRANSLATOR_H__
#define LUA_TRANSLATOR_H__

#include "base_translator.h"
#include "basedefine.h"
#include <fstream>

using namespace std;

class CLuaTranslator : public CBaseTranslator
{
public:
	CLuaTranslator();
	~CLuaTranslator();

public:
	void	TranslateProtocol();

private:
	void	OpenObjFile();
	void	CloseObjFile();

	void	AddFileDescToFile();
	void	AddUserClassDef();
	void	AddMsgBodyDef();

	void	InsertSeprator(ofstream& out);
	void	InitUserClass(const string& str_name, const MemItemVec& mem_set);
	void	GenarateObjNew(const string& str_name);
	void	GenarateObjPackFn(const string& str_name, const MemItemVec& mem_set);
	void	GenarateObjUnpackFn(const string& str_name, const MemItemVec& mem_set);

	void	GenaratePackObjCode(const string& val_name);
	void	GenaratePackArrayCode(const string& val_type, const string& val_name);
	void	GenaratePackValueCode(const string& val_type, const string& val_name, BaseType t);

	void	GenarateUnpackObjCode(const string& val_type, const string& val_name);
	void	GenarateUnpackValueCode(const string& val_type, const string& val_name, BaseType t);
	void	GenarateUnpackArrayCode(const string& val_type, const string& val_name);

	void	GenarateInterfaceFile(string& msgid);//生成接口文件和接口函数

	string	GetLuaFormat(BaseType t);

private:
	ofstream		out_file_;
};

#define LUA_IF_VALUE_NIL(name)\
	out_file_ << "\tif self." << name << " ~= nil then" << endl
#define LUA_END_IF\
	out_file_ << "\tend" << endl

#define LUA_CHECK_DECODE_POS\
	out_file_ << "\tif next > size then" << endl\
			  << "\t\treturn next" << endl\
			  << "\tend" << endl
#endif