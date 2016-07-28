#include <string.h>
#include "stat_utils.h"

void put_stat_value(struct stat_t *s, int32_t val) {
    if (0 == s) return;
    if (s->max_val < val) {
        s->max_val = val;
        if (s->history_max < val)
            s->history_max = val;
    }

    if (s->min_val > val) {
        s->min_val = val;
        if (s->history_min > val)
            s->history_min = val;
    }
}

void reset_stat_value(struct stat_t *s) {
    if (0 == s) return;

    memset(s, sizeof(struct stat_t), 0);
    s->history_max = INT32_MIN;
    s->max_val = INT32_MIN;
    s->history_min = INT32_MAX;
    s->min_val = INT32_MAX;
    s->mean_val = INT32_MIN;
}

void clear_stat_value(struct stat_t *s, int32_t min_val, int32_t max_val) {
    if (0 == s) return;

    s->max_val = max_val;
    s->min_val = min_val;
    s->mean_val = min_val;
}