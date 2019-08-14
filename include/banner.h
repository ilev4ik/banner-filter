#pragma once
#ifndef RAMBLER_BANNERS_BANNER_H
#define RAMBLER_BANNERS_BANNER_H

#include <unordered_set>
#include <ostream>

#include "banner_traits.h"

class banner : public eq_mixin<banner>
{
    using rank_t = int;
public:
    rank_t rank() const { return price; }
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
