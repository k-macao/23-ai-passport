#include "stock_data.h"

void stock_carousel_init(stock_carousel_t *c, size_t count)
{
    if (!c) return;
    c->current_index = 0;
    c->count = count;
}

size_t stock_carousel_next(stock_carousel_t *c)
{
    if (!c || c->count == 0) return 0;
    c->current_index = (c->current_index + 1) % c->count;
    return c->current_index;
}

size_t stock_carousel_prev(stock_carousel_t *c)
{
    if (!c || c->count == 0) return 0;
    c->current_index = (c->current_index + c->count - 1) % c->count;
    return c->current_index;
}
