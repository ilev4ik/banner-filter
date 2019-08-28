#pragma once
#ifndef RAMBLER_BANNERS_BANNER_TRAITS_H
#define RAMBLER_BANNERS_BANNER_TRAITS_H

#include <cstddef>

#include "banner.h"

template <typename T>
struct eq_mixin
{
    friend bool operator==(const T& lhs, const T& rhs) {
        return lhs.rank() == rhs.rank();
    }

    friend bool operator!=(const T& lhs, const T& rhs) {
        return lhs.rank() != rhs.rank();
    }
};

template <typename T>
struct banner_traits
{
    struct greater
    {
        bool operator()(const T& lhs, const T& rhs) const {
            return lhs.rank() > rhs.rank();
        }
    };

    struct non_equal {
        bool operator()(const T& lhs, const T& rhs) const {
            return lhs.rank() != rhs.rank();
        }
    };

};

#endif //RAMBLER_BANNERS_BANNER_TRAITS_H
