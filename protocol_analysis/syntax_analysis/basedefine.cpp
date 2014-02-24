#include "basedefine.h"
#include "filetitle_handler.h"
#include "namespace_handler.h"
#include "var_handler.h"
#include "typedef_handler.h"
#include "datatype_handler.h"
#include "message_handler.h"

CKeyWordSet::CKeyWordSet()
{
	for (int i = 0; i < KEYWORDNUM; i++)
	{
		string str_kword(g_keywords[i]);
		keywords_.insert(str_kword);
	}
}

CKeyWordSet::~CKeyWordSet()
{
}


bool CKeyWordSet::IsKeyWord(string& kwd)
{
	KeyWordSet::iterator itr = keywords_.find(kwd);
	if (itr != keywords_.end())
	{
		return true;
	}else
	{
		return false;
	}
}

CDataTypeSet::CDataTypeSet()
{
	for (int i = 0; i < BASEDATATYPENUM; i++)
	{
		string str_type(g_basedatatype[i]);
		datatype_.insert(str_type);
	}
}

CDataTypeSet::~CDataTypeSet()
{

}

void CDataTypeSet::AddDataType(string& str_type)
{
	datatype_.insert(str_type);
}

bool CDataTypeSet::IsContainType(const string& str_type)
{
	TypeSet::iterator itr = datatype_.find(str_type);
	if (itr != datatype_.end())
	{
		return true;
	} 
	else
	{
		return false;
	}
	
}

CBlockHandlerMap::CBlockHandlerMap()
{
	InitBlockHandlerMap();
}

CBlockHandlerMap::~CBlockHandlerMap()
{
	for (BlockHandlerMap::iterator itr = block_handler_.begin();
		itr != block_handler_.end(); itr++)
	{
		delete itr->second;
		itr->second = NULL;
	}
}

void CBlockHandlerMap::RegisterBlockHandler(string& key_word, CBaseBlockHandler* p_blockhandler)
{
	block_handler_.insert(make_pair(key_word, p_blockhandler));
}

void CBlockHandlerMap::RegisterBlockHandler(CBaseBlockHandler* p_blockhandler)
{
	block_handler_.insert(make_pair(p_blockhandler->GetKeyWord(), p_blockhandler));
}

//在初始化函数中注册需要处理的关键字和它的处理类
void CBlockHandlerMap::InitBlockHandlerMap()
{
	RegisterBlockHandler(new CFileTitleHandler);
	RegisterBlockHandler(new CNameSpaceHandler);
	RegisterBlockHandler(new CVarHandler);
	RegisterBlockHandler(new CTypedefHandler);
	RegisterBlockHandler(new CDatatypeHandler);
	RegisterBlockHandler(new CMessageDefHandler);
}

CBaseBlockHandler* CBlockHandlerMap::FetchKeyWordHandler(string& str_kword)
{
	BlockHandlerMap::iterator itr = block_handler_.find(str_kword);
	if (itr == block_handler_.end())
	{
		return NULL;
	}
	return itr->second;		
}

bool operator<(const CMemItem& l_mem,const CMemItem& r_mem)
{
	if (l_mem.m_name < r_mem.m_name)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CUserClassMap::AddUserClass(string& str_name, MemItemVec& desc)
{
	pair<UserClassDescMap::iterator, bool> ret = user_class_map_.insert(make_pair(str_name, desc));
	if (ret.second)
	{
		user_class_order_.push_back(str_name);
	}
}

bool CUserClassMap::IsUserDataType(const string& str_name)
{
	for (UserClassDescMapIte ite = user_class_map_.begin();
		ite != user_class_map_.end(); ite++)
	{
		if (ite->first == str_name)
		{
			return true;
		}
	}
	return false;
}

bool operator<(const CTypedefDesc& l, const CTypedefDesc& r)
{
	if (l.alias_ < r.alias_)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CTypedefSet::AddTypedef(string& str_type, string& alias) 
{
	CTypedefDesc typedef_desc(str_type, alias);
	typedef_set_.insert(typedef_desc);
}

string CTypedefSet::FindTypeAlias(string& str_type)
{
	for (TypeDefSet::iterator itr = typedef_set_.begin();
		itr != typedef_set_.end(); itr++)
	{
		if (itr->type_ == str_type)
		{
			return itr->alias_;
		}
	}
	return string();
}

string CTypedefSet::FindSrcTypeFromAlias(const string& str_alias)
{
	for (TypeDefSet::iterator itr = typedef_set_.begin();
		itr != typedef_set_.end(); itr++)
	{
		if (itr->alias_ == str_alias)
		{
			return itr->type_;
		}
	}
	return string();
}

void CFileDescMap::AddFileDesc(string& str_kword, string& content)
{
	filedesc_map_.insert(make_pair(str_kword, content));
}

string CFileDescMap::FindDescInfo(string& str_kword)
{
	FileDescMap::iterator itr = filedesc_map_.find(str_kword);
	if (itr != filedesc_map_.end())
	{
		return itr->second;
	}
	else
	{
		return string();
	}
}

void CVarDefMap::AddVarDef(string& str_vname, string& content)
{
	pair<VarDefMap::iterator, bool> ret = vardef_map_.insert(make_pair(str_vname, content));
	if (ret.second)
	{
		var_order_.push_back(str_vname);
	}
}

bool CVarDefMap::IsVarDefined(string& str_vname)
{
	if (vardef_map_.find(str_vname) != vardef_map_.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

string CVarDefMap::fetch_value(const string& str_vname)
{
	VarDefMap::iterator itr = vardef_map_.find(str_vname);
	if (itr == vardef_map_.end())
	{
		return string();
	}
	return itr->second;
}

bool operator<(const CMsgDefDesc& l, const CMsgDefDesc& r)//确定msg类型时已经检测了名字是否重定义
{
	if (l.msg_id_ < r.msg_id_ /*&& l.msg_cname_ < r.msg_cname_*/)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CMsgBodyDefMap::AddMsgDef(CMsgDefDesc& msg_def_desc, MemItemVec& msg_body_desc)
{
	pair<MsgBodyDefMap::iterator, bool> ret = msgbody_map_.insert(make_pair(msg_def_desc, msg_body_desc));
	if (ret.second)
	{
		msgbody_order_.push_back(msg_def_desc.msg_id_);
	}
}

bool CMsgBodyDefMap::IsMsgIDUsed(string& msg_id)
{
	MsgBodyDefMap::iterator itr = msgbody_map_.find(CMsgDefDesc(msg_id,string()));
	if (itr != msgbody_map_.end())
	{
		return true;
	}
	
	string id_value = GET_VARDEFMAP()->fetch_value(msg_id);
	if (id_value.empty())
	{
		return false;
	}

	string temp_id;
	for (itr = msgbody_map_.begin(); itr != msgbody_map_.end(); ++itr)
	{
		temp_id = GET_VARDEFMAP()->fetch_value(itr->first.msg_id_);
		if (temp_id == id_value)
		{
			return true;
		}
	}

	return false;
}

//判断msg_name是否是一个消息体名
bool CMsgBodyDefMap::IsMsgType(const string& msg_name)
{
	for (size_t i = 0; i < msgbody_order_.size(); i++)
	{
		if (msgbody_order_[i] == msg_name)
		{
			return true;
		}
	}
	return false;
}

void CMsgBodyDefMap::PrintSelfMap()
{
	for (MsgBodyDefMap::iterator ite = msgbody_map_.begin();
		ite != msgbody_map_.end(); ite++)
	{
		cout << ite->first.msg_id_ << " " << ite->first.msg_cname_ << endl;
		for (MemItemVec::iterator m_ite = ite->second.begin();
			m_ite != ite->second.end(); m_ite++)
		{
			m_ite->PrintSelf();
		}
	}
	cout << "-------" << endl;
}

const TypeDefSet& CVistor::GetTypedefSet()
{
	return GET_TYPEDEFSET()->typedef_set_;
}

const UserClassDescMap& CVistor::GetUserClassDescMap()
{
	return GET_USERCLASSMAP()->user_class_map_;
}

const UserClassOrder&	CVistor::GetUserClassOrder()
{
	return GET_USERCLASSMAP()->user_class_order_;
}

const VarDefMap& CVistor::GetVarMap()
{
	return GET_VARDEFMAP()->vardef_map_;
} 

const VarOrder& CVistor::GetVarOrder()
{
	return GET_VARDEFMAP()->var_order_;
}

const MsgBodyDefMap& CVistor::GetMsgBodyMap()
{
	return GET_MSGDEFMAP()->msgbody_map_;
}

const MsgBodyOrder& CVistor::GetMsgBodyOrder()
{
	return GET_MSGDEFMAP()->msgbody_order_;
}