#ifndef __RECIVER_UDP_HANDLER_H_
#define __RECIVER_UDP_HANDLER_H_

#include "revolver/base_event_handler.h"
#include "revolver/base_sock_dgram.h"
#include "revolver/base_bin_stream.h"
#include "revolver/base_singleton.h"
#include "reciver_adapter.h"

using namespace BASE;

class RecvUDPHandler : public CEventHandler
{
public:
	RecvUDPHandler();
	virtual ~RecvUDPHandler();

	bool			is_open() const;

	int32_t			open(const Inet_Addr& local_addr);
	int32_t			close();
	int32_t			send(BinStream& bin_strm, const Inet_Addr& remote_addr);

	BASE_HANDLER	get_handle() const;

	virtual int32_t	handle_input(BASE_HANDLER handle);
	virtual int32_t handle_output(BASE_HANDLER handle);
	virtual int32_t handle_close(BASE_HANDLER handle, ReactorMask close_mask);
	virtual int32_t handle_exception(BASE_HANDLER handle);

protected:
	BinStream		bin_strm_;		
	CSockDgram		sock_dgram_;
	ReciverAdapter	adapter_;
};

#define CREATE_RECV_UDP			CSingleton<RecvUDPHandler>::instance
#define RECV_UDP				CSingleton<RecvUDPHandler>::instance
#define DESTROY_RECV_UDP		CSingleton<RecvUDPHandler>::destroy

#endif

