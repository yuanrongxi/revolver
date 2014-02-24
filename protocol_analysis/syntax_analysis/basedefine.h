#ifndef KEY_WORD_SET_H_
#define KEY_WORD_SET_H_

#include "global.h"
#include "base_block_handler.h"
#include "singleton.h"
#include <set>
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

#define THROW_ERROR(line_num, err_str) \
	CBaseError err(line_num, err_str);\
	throw err\

#define DATA_TYPE	"datatype"
#define FILE_TITLE	"filetitle"
#define VAR			"var"
#define NAME_SPACE	"namespace"
#define TYPE_DEFINE "typedef"
#define MESSAGE		"message"

typedef vector<string>					StringArray;
typedef string::size_type				StrSizeType;

class CMemItem;
class CTypedefDesc;
class CMsgDefDesc;
class CBaseBlockHandler;

typedef set<string>						KeyWordSet;
typedef set<string>						TypeSet;
typedef map<string, CBaseBlockHandler*> BlockHandlerMap;
typedef set<CTypedefDesc>				TypeDefSet;
typedef vector<CMemItem>				MemItemVec;
typedef map<string, MemItemVec>			UserClassDescMap;
typedef map<string, string>				FileDescMap;
typedef map<string, string>				VarDefMap;
typedef map<CMsgDefDesc, MemItemVec>	MsgBodyDefMap;

typedef map<string, string>::const_iterator		StringMapIte;
typedef TypeDefSet::const_iterator				TypeDescSetIte;
typedef UserClassDescMap::const_iterator		UserClassDescMapIte;
typedef MsgBodyDefMap::const_iterator			MsgBodyDefMapIte;

typedef vector<string>	UserClassOrder;	//按照读取类名的顺序存放类名
typedef vector<string>	VarOrder;		//按照读取变量的名字存放变量名
typedef vector<string>	MsgBodyOrder;	//按照读取消息的名字存放msgid

class CKeyWordSet
{
public:
	CKeyWordSet();
	virtual ~CKeyWordSet();

public:
	bool		IsKeyWord(string& kwd);

private:
	KeyWordSet	keywords_;
};

class CDataTypeSet
{
public:
	CDataTypeSet();
	virtual ~CDataTypeSet();

public:
	void	AddDataType(string& str_type);
	bool	IsContainType(const string& str_type);

private:
	TypeSet	datatype_;
};

class CBlockHandlerMap
{
public:
	CBlockHandlerMap();
	virtual ~CBlockHandlerMap();

public:
	void				InitBlockHandlerMap();
	CBaseBlockHandler*	FetchKeyWordHandler(string& str_kword);

private:
	void	RegisterBlockHandler(string& key_word, CBaseBlockHandler* p_blockhandler);
	void	RegisterBlockHandler(CBaseBlockHandler* p_blockhandler);

private:
	BlockHandlerMap		block_handler_;
};

class CMemItem
{
public:
	CMemItem(){}
	~CMemItem(){}

public:
	void PrintSelf()
	{
		cout << m_type << " " << m_name << " = " << m_value << endl;
	}

public:
	string m_type;
	string m_name;
	string m_value;
};

bool operator<(const CMemItem& l_mem,const CMemItem& r_mem);

class CUserClassMap
{
public:
	friend class CVistor;

	CUserClassMap(){}
	~CUserClassMap(){}

public:	
	void	AddUserClass(string& str_name, MemItemVec& desc);
	bool	IsUserDataType(const string& str_name);
	
private:
	UserClassDescMap	user_class_map_;
	UserClassOrder		user_class_order_;
};

class CTypedefDesc
{
public:
	CTypedefDesc(string& str_type, string& str_alias) 
			: type_(str_type), alias_(str_alias)
	{

	}

public:
	string type_;
	string alias_;
};

bool operator<(const CTypedefDesc& l, const CTypedefDesc& r);

class CTypedefSet
{
public:
	friend class CVistor;

	CTypedefSet(){}
	~CTypedefSet(){}

public:
	void		AddTypedef(string& str_type, string& alias);
	string		FindTypeAlias(string& str_type);
	string		FindSrcTypeFromAlias(const string& str_alias);
 
private:
	TypeDefSet	typedef_set_;
};

class CFileDescMap
{
public:
	CFileDescMap(){}
	~CFileDescMap(){}

public:
	void		AddFileDesc(string& str_kword, string& content);
	string		FindDescInfo(string& str_kword);

private:
	FileDescMap	filedesc_map_;
};

class CVarDefMap
{
public:
	friend class CVistor;

	CVarDefMap(){}
	~CVarDefMap(){}

public:
	void		AddVarDef(string& str_vname, string& content);
	bool		IsVarDefined(string& str_vname);
	string		fetch_value(const string& str_vname);

private:
	VarDefMap	vardef_map_;
	VarOrder	var_order_;
};

class CMsgDefDesc
{
public:
	CMsgDefDesc(string& str_msgid, string& str_msgname) 
				: msg_id_(str_msgid), msg_cname_(str_msgname)
	{
	}

public:
	string msg_id_;
	string msg_cname_;
};

bool operator<(const CMsgDefDesc& l, const CMsgDefDesc& r);

class CMsgBodyDefMap
{
public:
	friend class CVistor;

	CMsgBodyDefMap(){}
	~CMsgBodyDefMap(){}

public:
	void			AddMsgDef(CMsgDefDesc& msg_def_desc, MemItemVec& msg_body_desc);
	bool			IsMsgIDUsed(string& msg_id);
	bool			IsMsgType(const string& msg_name);

	//for debug
	void			PrintSelfMap();

private:
	MsgBodyDefMap	msgbody_map_;
	MsgBodyOrder	msgbody_order_;
};

class CVistor
{
public:
	static const TypeDefSet&		GetTypedefSet();

	static const UserClassDescMap&	GetUserClassDescMap();
	static const UserClassOrder&	GetUserClassOrder();

	static const VarDefMap&			GetVarMap(); 
	static const VarOrder&			GetVarOrder();

	static const MsgBodyDefMap&		GetMsgBodyMap();
	static const MsgBodyOrder&		GetMsgBodyOrder();
};

#define CREATE_DATATYPESET		CSingleton<CDataTypeSet>::CreateInstance
#define GET_DATATYPESET			CSingleton<CDataTypeSet>::GetInstance
#define DESTORY_DATATYPESET		CSingleton<CDataTypeSet>::DestoryInstance

#define CREATE_KEYWORDSET		CSingleton<CKeyWordSet>::CreateInstance
#define GET_KEYWORDSET			CSingleton<CKeyWordSet>::GetInstance
#define DESTORY_KEYWORDSET		CSingleton<CKeyWordSet>::DestoryInstance

#define CREATE_BLOCKHANDLERMAP	CSingleton<CBlockHandlerMap>::CreateInstance
#define GET_BLOCKHANDLERMAP		CSingleton<CBlockHandlerMap>::GetInstance
#define DESTORY_BLOCKHANDLERMAP CSingleton<CBlockHandlerMap>::DestoryInstance

#define CREATE_USERCLASSMAP		CSingleton<CUserClassMap>::CreateInstance
#define GET_USERCLASSMAP		CSingleton<CUserClassMap>::GetInstance
#define DESTORY_USERCLASSMAP	CSingleton<CUserClassMap>::DestoryInstance

#define CREATE_TYPEDEFSET		CSingleton<CTypedefSet>::CreateInstance
#define GET_TYPEDEFSET			CSingleton<CTypedefSet>::GetInstance
#define DESTORY_TYPEDEFSET		CSingleton<CTypedefSet>::DestoryInstance

#define CREATE_FILEDESCMAP		CSingleton<CFileDescMap>::CreateInstance
#define GET_FILEDESCMAP			CSingleton<CFileDescMap>::GetInstance
#define DESTORY_FILEDESCMAP		CSingleton<CFileDescMap>::DestoryInstance

#define CREATE_VARDEFMAP		CSingleton<CVarDefMap>::CreateInstance
#define GET_VARDEFMAP			CSingleton<CVarDefMap>::GetInstance
#define DESTORY_VARDEFMAP		CSingleton<CVarDefMap>::DestoryInstance

#define CREATE_MSGDEFMAP		CSingleton<CMsgBodyDefMap>::CreateInstance
#define GET_MSGDEFMAP			CSingleton<CMsgBodyDefMap>::GetInstance
#define DESTORY_MSGDEFMAP		CSingleton<CMsgBodyDefMap>::DestoryInstance

#endif //KEY_WORD_SET_H_