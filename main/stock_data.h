#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    const char *market;     // e.g. "A-SHARE"
    const char *name;       // e.g. "SHANGHAI (000001)"
    const char *points_str; // e.g. "3,885.33"
    const char *chg_str;    // e.g. "-0.07%"
    bool is_up;
} stock_item_t;

typedef struct {
    size_t current_index;
    size_t count;
} stock_carousel_t;

void stock_carousel_init(stock_carousel_t *c, size_t count);
size_t stock_carousel_next(stock_carousel_t *c);
size_t stock_carousel_prev(stock_carousel_t *c);
