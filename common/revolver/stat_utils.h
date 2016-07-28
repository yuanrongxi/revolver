#ifndef __STAT_UTILS_H__
#define __STAT_UTILS_H__

#include <stdint.h>
#include <limits.h>

struct  stat_t {
    int32_t max_val;
    int32_t history_min;
    int32_t min_val;
    int32_t history_max;
    int32_t mean_val;
};

void put_stat_value(struct stat_t *s, int32_t val);
void clear_stat_value(struct stat_t *s, int32_t min_val = INT32_MAX, int32_t max_val = INT32_MIN);
void reset_stat_value(struct stat_t *s);

#endif // __STAT_UTILS_H__
