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
};

template <typename T>
struct banner_traits
{
    using sum_t = typename T::rank_t;

    struct greater
    {
        bool operator()(const T& lhs, const T& rhs) const {
            return lhs.rank() > rhs.rank();
        }
    };

    struct summator
    {
        sum_t operator()(sum_t acc, const T& b) {
            return acc + b.rank();
        }
    };

    struct calc_rv
    {
        struct greater
        {
            bool operator()(const calc_rv& lhs, const calc_rv& rhs) {
                return lhs.sum > rhs.sum;
            }
        };

        struct non_equal
        {
            bool operator()(const calc_rv& lhs, const calc_rv& rhs) {
                return lhs.sum != rhs.sum;
            }
        };

        sum_t sum;
        std::size_t offset;
        int id;
    };
};

#endif //RAMBLER_BANNERS_BANNER_TRAITS_H
