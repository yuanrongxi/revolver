#include "rudp/rudp_log_macro.h"

//#ifdef __ANDROID__
//extern const char* ex_log_path;
//SingleRotatedLogStream rudp_log(ex_log_path, "rudp", SingleLogStream::trace);
//SingleRotatedLogStream rudp_recv_log(ex_log_path, "rudp_recv", SingleLogStream::trace);
//SingleRotatedLogStream rudp_send_log(ex_log_path, "rudp_send", SingleLogStream::trace);
//#else
SingleRotatedLogStream rudp_log("rudp", SingleLogStream::trace);
SingleRotatedLogStream rudp_recv_log("rudp_recv", SingleLogStream::trace);
SingleRotatedLogStream rudp_send_log("rudp_send", SingleLogStream::trace);
SingleRotatedLogStream rudp_trace_log("rudp_trace", SingleLogStream::trace);
//#endif

void change_rudp_log_path(const char* file_path) {
    if (0 == file_path) return;
    rudp_log.change_path(file_path);
    rudp_log.set_max_file_cnt(20);
    rudp_recv_log.change_path(file_path);
    rudp_recv_log.set_max_file_cnt(20);
    rudp_send_log.change_path(file_path);
    rudp_send_log.set_max_file_cnt(20);
    rudp_trace_log.change_path(file_path);
    rudp_trace_log.set_max_file_cnt(20);
}