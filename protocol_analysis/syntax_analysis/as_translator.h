#ifndef AS_TRANSLATOR_H__
#define AS_TRANSLATOR_H__

#include "base_translator.h"
#include "basedefine.h"
#include <fstream>

using namespace std;

class CASTranslator : public CBaseTranslator
{
public:
	CASTranslator(){}
	~CASTranslator(){}

public:
	void	TranslateProtocol();
private:
	ofstream& open_file(const string& fname);

	void	generate_package(const MemItemVec& mem_vec, string cname, string msgid = "");

private:
	ofstream out_file_;
};

class ICodeGenerator
{
public:
	ICodeGenerator(){}
	virtual ~ICodeGenerator(){}

public:
	virtual void code_generate(ofstream& o_file, const MemItemVec& mem_vec) = 0;
};

class ASBaseGenerator : public ICodeGenerator
{
public:
	ASBaseGenerator(ICodeGenerator* generator = NULL) : generator_(generator){}
	virtual ~ASBaseGenerator(){if(generator_ != NULL){delete generator_;}}

public:
	virtual void code_generate(ofstream& o_file, const MemItemVec& mem_vec){}

protected:
	ICodeGenerator* generator_;
};

class ASPacketGenerator : public ASBaseGenerator
{
public:
	ASPacketGenerator(const string& cname, const string& id = "", ICodeGenerator* generator = NULL)
		: ASBaseGenerator(generator), msg_id_(id), class_name_(cname){}
	virtual ~ASPacketGenerator(){}

public:
	virtual void code_generate(ofstream& o_file, const MemItemVec& mem_vec);

	void	set_generate_msg(string& msgid){msg_id_ = msgid;}
	void	set_class_name(string& cname){class_name_ = cname;}

private:
	void	func_constructor(ofstream& o_file, const MemItemVec& mem_vec);
	void	generate_members(ofstream& o_file, const MemItemVec& mem_vec);

	string	generate_memtype(const string& m_type);
private:
	string	msg_id_;
	string	class_name_;
};

class ASClassFunctions : public ASBaseGenerator
{
public:
	ASClassFunctions(ICodeGenerator* generator = NULL)
		: ASBaseGenerator(generator){}
	virtual ~ASClassFunctions(){}

public:
	virtual void code_generate(ofstream& o_file, const MemItemVec& mem_vec);

private:
	void	func_pack(ofstream& o_file, const MemItemVec& mem_vec);
	void	func_unpack(ofstream& o_file, const MemItemVec& mem_vec);

	void	generate_packcode(ofstream& o_file, const string& type, const string& name, const string& space);
	void	packstring_code(ofstream& o_file, const string& name, const string& space);
	void	packarray_code(ofstream& o_file, const string& type, const string& name, const string& space);

	void	generate_unpackcode(ofstream& o_file, const string& type, const string& name, const string& space);
	void	unpackstring_code(ofstream& o_file, const string& name, const string& space);
	void	unpackarray_code(ofstream& o_file, const string& type, const string& name, const string& space);
};

string	get_as_type(string& src_type);
string  get_as_type(BaseType base_type);

#endif