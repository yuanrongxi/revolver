#ifndef __NET_STAT_H__
#define __NET_STAT_H__

#include <stdint.h>

class NetStat {
public:
    NetStat() {
        reset();
    }

    inline void reset() {
        _in_pkt_cnt = _out_pkt_cnt = 0;
        _in_bytes = _out_bytes = 0;
    }

    inline void push_in_data(uint32_t len) {
        _in_pkt_cnt++;
        _in_bytes += len;
    }
    inline void push_out_data(uint32_t len) {
        _out_pkt_cnt++;
        _out_bytes += len;
    }
public:
    uint32_t _in_pkt_cnt, _out_pkt_cnt;
    uint32_t _in_bytes, _out_bytes;
};


#endif // __NET_STAT_H__
