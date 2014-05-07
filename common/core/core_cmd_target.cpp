#include "revolver/base_packet.h"
#include "core/core_cmd_target.h"
#include "core/core_msg_.h"

BASE_NAMESPACE_BEGIN_DECL

union MessageMapFunctions
{
	TARGET_CALL pfn;  

	void	(ICmdTarget::*pfn_v_p_v)(CBasePacket*);
	int32_t (ICmdTarget::*pfn_i_p_v)(CBasePacket*);
	int32_t (ICmdTarget::*pfn_i_p_u)(CBasePacket*, uint32_t);
	int32_t (ICmdTarget::*pfn_i_p_uC)(CBasePacket*, uint32_t, CConnection*);
	int32_t (ICmdTarget::*pfn_i_p_a)(CBasePacket*,  uint32_t, const Inet_Addr&);
};

int32_t CoreDispatchCmdMsg(ICmdTarget* target, CMD_MESSAGE_ENTRY* entity, CBasePacket *packet,
						   uint32_t sid, CConnection *connection)
{
	int32_t ret = -1;
	if(entity == NULL || target == NULL)
	{
		return ret;
	}
	
	MessageMapFunctions mfn;
	mfn.pfn = entity->fun;

	switch(entity->sig)
	{
	case CoreSig_v_P_v:
		{
			(target->*mfn.pfn_v_p_v)(packet);
			ret = 0;
		}
		break;

	case CoreSig_i_P_v:
		{
			ret = (target->*mfn.pfn_i_p_v)(packet);
		}
		break;

	case CoreSig_i_P_u:
		{
			ret = (target->*mfn.pfn_i_p_u)(packet, sid);
		}
		break;

	case CoreSig_i_P_u_C:
		{
			ret = (target->*mfn.pfn_i_p_uC)(packet, sid, connection);
		}
		break;
	}

	return ret;
}

int32_t CoreDispatchCmdMsg(ICmdTarget* target, CMD_MESSAGE_ENTRY* entity, CBasePacket *packet,
						   uint32_t sid, const Inet_Addr& remote_addr)
{
	int32_t ret = -1;
	if(entity == NULL || target == NULL)
	{
		return ret;
	}

	MessageMapFunctions mfn;
	mfn.pfn = entity->fun;

	switch(entity->sig)
	{
	case CoreSig_i_P_A:
		{
			ret = (target->*mfn.pfn_i_p_a)(packet, sid, remote_addr);
		}
		break;
	}

	return ret;
}

ICmdTarget::ICmdTarget()
{

}

ICmdTarget::~ICmdTarget()
{

}

int32_t ICmdTarget::on_event(uint32_t msg_id, uint32_t sid, CBasePacket* packet, CConnection* connection)
{	
	CTargetMessageManager *msg_manager = get_message_map();
	CMD_MESSAGE_ENTRY* entity = msg_manager->get_callback(msg_id);	
	
	return CoreDispatchCmdMsg(this, entity, packet, sid, connection);
}

int32_t ICmdTarget::on_event(uint32_t msg_id, uint32_t sid, CBasePacket* packet, const Inet_Addr& remote_addr)
{
	CTargetMessageManager *msg_manager = get_message_map();
	CMD_MESSAGE_ENTRY* entity = msg_manager->get_callback(msg_id);	

	return CoreDispatchCmdMsg(this, entity, packet, sid, remote_addr);
}

////////////////////////////////////////////////////////////////////////////////////


CTargetMessageManager::CTargetMessageManager()
{

}

CTargetMessageManager::~CTargetMessageManager()
{
	message_map_.clear();
}

void CTargetMessageManager::register_callback(uint32_t msg_id, uint32_t sig, TARGET_CALL fun)
{
	CMD_MESSAGE_ENTRY entity;
	entity.fun = fun;
	entity.msg_id = msg_id;
	entity.sig = sig;

	message_map_[msg_id] = entity;
}

CMD_MESSAGE_ENTRY*	CTargetMessageManager::get_callback(uint32_t msg_id)
{
	CMD_MESSAGE_MAP::iterator it = message_map_.find(msg_id);
	if(it != message_map_.end())
	{
		return &(it->second);
	}
	else
	{
		return NULL;
	}
}

BASE_NAMESPACE_END_DECL
