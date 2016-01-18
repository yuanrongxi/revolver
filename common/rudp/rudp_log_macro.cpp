#include "rudp/rudp_log_macro.h"

#ifdef __ANDROID__
extern const char* ex_log_path;
SingleLogStream rdup_log(ex_log_path, "rudp", SingleLogStream::debug);
SingleLogStream rdup_recv_log(ex_log_path, "rudp_recv", SingleLogStream::debug);
SingleLogStream rdup_send_log(ex_log_path, "rudp_send", SingleLogStream::debug);
#else
SingleLogStream rdup_log("rudp", SingleLogStream::debug);
SingleLogStream rdup_recv_log("rudp_recv", SingleLogStream::debug);
SingleLogStream rdup_send_log("rudp_send", SingleLogStream::debug);
#endif