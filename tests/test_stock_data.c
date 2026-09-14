#include <assert.h>
#include "stock_data.h"

int main(void)
{
    stock_carousel_t carousel;
    stock_carousel_init(&carousel, 3);
    assert(carousel.current_index == 0);
    assert(carousel.count == 3);

    assert(stock_carousel_next(&carousel) == 1);
    assert(stock_carousel_next(&carousel) == 2);
    assert(stock_carousel_next(&carousel) == 0);

    assert(stock_carousel_prev(&carousel) == 2);
    assert(stock_carousel_prev(&carousel) == 1);
    assert(stock_carousel_prev(&carousel) == 0);

    return 0;
}
