#pragma once
#ifndef RAMBLER_BANNERS_BANNER_EX_H
#define RAMBLER_BANNERS_BANNER_EX_H

#include "banner.h"
#include "banner_traits.h"

#include "tuple_utils.h"

class banner_ex : public banner, public eq_mixin<banner_ex>
{
    using rank_t = std::tuple<int, int, int, int, int>;
    rank_t rank() const { return std::make_tuple(price, popularity, pxm_bytes, area(), perimeter()); }

public:
    friend class banner_traits<banner_ex>;
    using banner::banner;
    banner_ex(int id, int pr, int pop)
            : banner(id, pr), popularity(pop)
    {}

    int popularity = -1;
    int pxm_bytes = -1;
    int width = -1;
    int height = -1;

    int area() const {
        return width * height;
    }

    int perimeter() const {
        return 2*(width + height);
    }

    // ...other metrics to prioritize

    friend std::ostream& operator<<(std::ostream& os, const banner_ex& b) {
        return os << '{' << b.adv_id << ", " << b.price << ", " << b.popularity << "..." <<'}';
    }
};


#endif //RAMBLER_BANNERS_BANNER_EX_H
