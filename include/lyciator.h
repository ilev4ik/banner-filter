#pragma once
#ifndef RAMBLER_BANNERS_LYCIATOR_H
#define RAMBLER_BANNERS_LYCIATOR_H

#include <random>
#include <unordered_map>

#include "simple_thread_pool.h"
#include "hash_back_inserter.h"

#include "banner_traits.h"
#include "filter.h"

template<class C, class T, class Compare>
inline void insert_sorted_vecs(C& c, const T& vec, Compare comp) {
    auto pos = std::lower_bound(c.begin(), c.end(), vec[0], comp);
    c.insert(pos, vec.begin(), vec.end());
}

template <typename BT>
class lyciator
{
    using compare_greater = typename banner_traits<BT>::greater;
    using compare_non_equal = typename banner_traits<BT>::non_equal;

    template <typename T>
    friend std::vector<T> auction(std::vector<T> banners, std::size_t lots, filter<T> banner_filter);

    // precondition: banners is not empty
    void find_best_max_banners(std::vector<BT>& banners) noexcept {
        // sort descending by rank()
        std::sort(banners.begin(), banners.end(), compare_greater{});

        // find first banner which rank() != max
        auto found_it = std::adjacent_find(banners.begin(), banners.end(), compare_non_equal{});
        auto from_it = found_it == banners.end() ? found_it : std::next(found_it);

        // erase the remaining ones
        banners.erase(from_it, banners.end());
    }

    template <typename It>
    It choose_uniformly(It left_it, It right_it)
    {
        // равновероятно выбрать из множества равнопредпочитаемых списков баннеров
        const auto d = std::max((ptrdiff_t )0, std::distance(left_it, right_it) - 1);
        std::uniform_int_distribution<std::size_t> dis(0, d);
        return std::next(left_it, dis(gen));
    }

    explicit lyciator(std::vector<BT>&& banners)
            : rd()
            , gen(rd())
            , m_banners(std::move(banners))
    {}

    void slice_bests_concurrently(std::unordered_map<int, std::vector<BT>>& by_adv_id)
    {

        // thread_pool с оптимальным количеством потоков-воркеров
        simple_thread_pool thread_pool(std::thread::hardware_concurrency());
        for (auto&& kv : by_adv_id) {
            thread_pool.enqueue([this, &kv]() { return find_best_max_banners(kv.second); });
        }
        thread_pool.wait_all();
    }

    std::vector<BT> run_auction(std::size_t lots, filter<BT>&& banner_filter)
    {
        if (lots == 0) throw std::invalid_argument("lots parameter have to be positive");

        // move banners satisfying filter and group by id into hash table
        std::unordered_map<int, std::vector<BT>> by_adv;
        std::copy_if(
                std::make_move_iterator(m_banners.begin()),
                std::make_move_iterator(m_banners.end()),
                hash_back_inserter(by_adv),
                std::move(banner_filter)
                );

        if (by_adv.empty()) return {};

        // remove non-best banners
        // remaining: 1..* (equal) best banners
        slice_bests_concurrently(by_adv);

        std::vector<BT> temp;
        // at least reserve groups number size
        temp.reserve(by_adv.size());
        for (auto&& kv : by_adv)
        {
            // insert banner in desc priority with repetitions
            // e.g. (grouped by id and rank()): 5, 5, 5, 5, 5, 4, 4, 4, 4, 2, 2, 2
            insert_sorted_vecs(temp, kv.second, compare_greater{});
        }

        // choose uniformly the random best one
        std::vector<BT> rv;
        rv.reserve(by_adv.size());

        // choose uniformly banner from each group
        // (5, 5, 5*, 5, 5), (4*, 4, 4, 4), (2, 2, 2*)
        // push into "return value"
        auto start = temp.begin();
        while (start != temp.end() && rv.size() != lots)
        {
            auto found_it = std::adjacent_find(start, temp.end(),
                    [](const BT& lhs, const BT& rhs)
            {
                return lhs.adv_id != rhs.adv_id;
            });
            auto to_it = found_it == temp.end() ? found_it : std::next(found_it);
            rv.push_back(*choose_uniformly(start, to_it));
            start = to_it;
        }
        return rv;
    }

    std::random_device rd;
    std::mt19937 gen;
    std::vector<BT> m_banners;
};

template <typename BT>
std::vector<BT> auction(std::vector<BT> banners, std::size_t lots, filter<BT> banner_filter = {})
{
    lyciator<BT> lyc(std::move(banners));
    return lyc.run_auction(lots, std::move(banner_filter));;
}

#endif //RAMBLER_BANNERS_LYCIATOR_H
