#pragma once
#ifndef RAMBLER_BANNERS_BANNER_EX_H
#define RAMBLER_BANNERS_BANNER_EX_H

#include "banner.h"
#include "banner_traits.h"

#include "tuple_utils.h"

struct banner_ex : public eq_mixin<banner_ex>
{
    using rank_t = std::tuple<int, int, int, int, int>;
    rank_t rank() const { return std::make_tuple(price, popularity, pxm_bytes, area(), perimeter()); }
    friend class banner_traits<banner_ex>;
    banner_ex(int id, std::size_t pr, int pop)
        : adv_id(id), price(pr), popularity(pop)
    {}

    banner_ex(int id, std::size_t pr, const std::string& c, int pop)
            : adv_id(id), price(pr), popularity(pop)
    {
        countries.insert(c);
    }

    int adv_id;
    std::size_t price;
    int popularity = 0;
    int pxm_bytes = 0;
    int width = 0;
    int height = 0;
    std::unordered_set<std::string> countries;

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
