#include "core/core_message_map_decl.h"

BASE_NAMESPACE_BEGIN_DECL

CMessageMapDecl::CMessageMapDecl()
{

}

CMessageMapDecl::~CMessageMapDecl()
{
	clear();
}

void CMessageMapDecl::clear()
{
	for(CoreMessageMap::iterator it = msg_map_.begin(); it != msg_map_.end(); it ++)
	{
		//delete it->second是否存在内存泄露？？可能要在CBasePacket增加个release_self的虚接口
		if(it->second != NULL)
		{
			it->second->release_self();
		}
	}

	msg_map_.clear();
}

CBasePacket* CMessageMapDecl::gain_message(uint32_t msg_id)
{
	CoreMessageMap::iterator it = msg_map_.find(msg_id);
	if(it != msg_map_.end())
	{
		return it->second;
	}
	else
	{
		return NULL;
	}
}

void CMessageMapDecl::register_message(uint32_t msg_id, CBasePacket* msg_obj)
{
	if(msg_obj == NULL)
		return;

	CoreMessageMap::iterator it = msg_map_.find(msg_id);
	if(it != msg_map_.end())
	{
		msg_obj->release_self();
	}
	else
	{
		msg_map_[msg_id] = msg_obj;
	}
}


BASE_NAMESPACE_END_DECL

