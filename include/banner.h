#pragma once
#ifndef RAMBLER_BANNERS_BANNER_H
#define RAMBLER_BANNERS_BANNER_H

#include "banner_traits.h"
#include <unordered_set>
#include <ostream>

class banner
{
    using rank_t = int;
    rank_t rank() const { return price; }
public:
    friend class banner_traits<banner>;
    banner(int id, int pr) : adv_id(id), price(pr) {}
    banner(int id, int pr, const std::string& country)
            : adv_id(id), price(pr)
    {
        countries.insert(country);
    }

    int adv_id;
    rank_t price;
    std::unordered_set<std::string> countries;

    friend std::ostream& operator<<(std::ostream& os, const banner& b) {
        return os << '{' << b.adv_id << ", " << b.price << '}';
    }
};

#endif //RAMBLER_BANNERS_BANNER_H
