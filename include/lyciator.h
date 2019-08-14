#pragma once
#ifndef RAMBLER_BANNERS_LYCIATOR_H
#define RAMBLER_BANNERS_LYCIATOR_H

#include <random>
#include <unordered_map>

#include "simple_thread_pool.h"
#include "hash_back_inserter.h"

#include "banner_traits.h"
#include "filter.h"

template <typename BT>
class lyciator
{
    using calc_rv_t = typename banner_traits<BT>::calc_rv;
    using compare_greater = typename banner_traits<BT>::greater;
    using summator = typename banner_traits<BT>::summator;
    using sum_t = typename banner_traits<BT>::sum_t;
    using calc_it_t = typename std::vector<calc_rv_t>::iterator;

    template <typename T>
    friend std::vector<T> auction(std::vector<T> banners, std::size_t lots, filter<T> banner_filter);

    // сортируется переданный массив: & banners
    // offset -- индекс последнего баннера после сортировки
    calc_rv_t find_best_n_banners(std::size_t lots, std::vector<BT>& banners) const noexcept {
        // precondition: banners is not empty

        // кол-во лотов (выходных баннеров)
        const auto pivot_offset = std::min(lots, banners.size());
        auto to_it = std::next(banners.begin(), pivot_offset);

        // нахождение N самых дорогих баннеров + их сумма
        std::partial_sort(banners.begin(), to_it, banners.end(), compare_greater{});
        const auto max_bounded_price = std::accumulate(banners.begin(), to_it, sum_t{}, summator{});

        return {max_bounded_price, pivot_offset, banners[0].adv_id};
    }

    explicit lyciator(std::vector<BT>&& banners)
            : rd()
            , gen(rd())
            , m_banners(std::move(banners))
    {}

    auto make_filter(filter<BT>&& banner_filter)
    -> typename std::vector<BT>::iterator
    {
        return std::remove_if(m_banners.begin(), m_banners.end(), std::move(banner_filter));
    }

    auto group_by_id(typename std::vector<BT>::iterator end)
    -> std::unordered_map<int, std::vector<BT>>
    {
        // разбить на группы по adv_id
        std::unordered_map<int, std::vector<BT>> by_adv;
        std::move(m_banners.begin(), end, hash_back_inserter(by_adv));
        return by_adv;
    }

    auto find_best_concurrently(std::unordered_map<int, std::vector<BT>>& by_adv_id, std::size_t lots) const
    -> std::vector<calc_rv_t>
    {
        std::vector<std::future<calc_rv_t>> max_prices;
        {
            // thread_pool с оптимальным количеством потоков-воркеров
            simple_thread_pool thread_pool(std::thread::hardware_concurrency());

            // решение сути задачи
            // отправить на выделенные потоки задачи на подсчёт суммы цен для одинаковых adv_id
            for (auto&& kv : by_adv_id) {
                max_prices.push_back(thread_pool.enqueue([this, lots, &kv]() {
                    return find_best_n_banners(lots, kv.second);
                }));
            }
        }

        // извлечь данные из фьючерсов
        std::vector<calc_rv_t> results;
        std::transform(max_prices.begin(), max_prices.end(), std::back_inserter(results),
                       [](std::future<calc_rv_t>& f)
                       {
                           return f.get();
                       });

        return results;
    }

    calc_it_t prioritize(std::vector<calc_rv_t>& results) const
    {
        // найти списки баннеров с самым высоким приоритетом
        std::sort(results.begin(), results.end(), typename calc_rv_t::greater{});
        return std::adjacent_find(results.begin(), results.end(), typename calc_rv_t::non_equal{});;
    }

    calc_it_t choose_uniformly(calc_it_t left_it, calc_it_t right_it)
    {
        // равновероятно выбрать из множества равнопредпочитаемых списков баннеров
        const auto d = std::max((ptrdiff_t)0, std::distance(left_it, right_it) - 1);
        std::uniform_int_distribution<std::size_t> dis(0, d);
        return std::next(left_it, dis(gen));
    }

    std::vector<BT> run_auction(std::size_t lots, filter<BT>&& banner_filter)
    {
        if (lots == 0) throw std::invalid_argument("lots parameter have to be positive");
        std::vector<BT> rv;

        // процесс аукциона
        auto filtered_banner_end = make_filter(std::move(banner_filter));
        if (std::distance(m_banners.begin(), filtered_banner_end)) {
            auto by_adv_id = group_by_id(filtered_banner_end);
            auto max_prices = find_best_concurrently(by_adv_id, lots);
            const auto right_it = prioritize(max_prices);
            auto best_it = choose_uniformly(max_prices.begin(), right_it);

            // переместить в возвращаемый результат
            const auto &cont = by_adv_id[best_it->id];
            auto from_it = cont.cbegin();
            auto to_it = std::next(cont.cbegin(), best_it->offset);
            std::move(from_it, to_it, std::back_inserter(rv));
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
