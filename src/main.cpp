#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <future>
#include <random>

#include "banner_traits.h"
#include "tuple_utils.h"
#include "hash_back_inserter.h"
#include "simple_thread_pool.h"

struct banner
{
    using rank_t = int;
    banner(int id, int pr) : adv_id(id), price(pr) {}
	banner(int id, int pr, const std::string& country)
	    : adv_id(id), price(pr)
	{
		countries.insert(country);
	}

    rank_t rank() const { return price; }

	int adv_id;
	rank_t price;
	std::unordered_set<std::string> countries;

    friend std::ostream& operator<<(std::ostream& os, const banner& b) {
        return os << '{' << b.adv_id << ", " << b.price << '}';
    }
};

struct banner_ex : banner
{
    using banner::banner;
    using rank_t = std::tuple<int, int, int, int, int>;

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

    rank_t rank() const {
        return std::make_tuple(price, popularity, pxm_bytes, area(), perimeter());
    }

    friend std::ostream& operator<<(std::ostream& os, const banner_ex& b) {
        return os << '{' << b.adv_id << ", " << b.price << ", " << b.popularity << "..." <<'}';
    }
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
	os << '[';
	for (auto&& elem : vec) {
		os << elem << ", ";
	}
	os << ']';
	return os;
}

template <typename BT>
struct max_sum_price_block
{
    using compare_greater = typename banner_traits<BT>::greater;
    using summator = typename banner_traits<BT>::summator;
    using sum_t = typename banner_traits<BT>::sum_t;
    using calc_rv_t = typename banner_traits<BT>::calc_rv;

	explicit max_sum_price_block(std::size_t lots): max_sum_price_block(lots, delegate_tag{})
	{
	    if (m_lots < 1) throw std::logic_error("lots count should at least one");
	}

	// сортируется переданный массив: & banners
	// offset -- индекс последнего баннера после сортировки
    calc_rv_t operator()(std::vector<BT>& banners) noexcept {
        // precondition: banners is not empty

        // кол-во лотов (выходных баннеров)
        const auto pivot_offset = std::min(m_lots, banners.size());
        auto to_it = std::next(banners.begin(), pivot_offset);

        // нахождение N самых дорогих баннеров + их сумма
        std::partial_sort(banners.begin(), to_it, banners.end(), compare_greater{});
        const auto max_bounded_price = std::accumulate(banners.begin(), to_it, sum_t{}, summator{});

        return {max_bounded_price, pivot_offset, banners[0].adv_id};
	}

private:
    // delegating cons; enabled possibility of throwing from cons
    struct delegate_tag {};
    max_sum_price_block(std::size_t lots, delegate_tag): m_lots(lots) {};

private:
	std::size_t m_lots;
};

template <typename T>
class filter
{
public:
    using pred_type = std::function<bool(const T& obj)>;
    void add(pred_type&& p) {
        m_preds.emplace_back(std::move(p));
    }

    bool operator()(const T& obj) const {
        return apply(obj);
    }

private:
    bool apply(const T& obj) const
    {
        // проверка объекта на условия всех предикатов фильтра
        auto check_pred = [obj](const pred_type& pred) { return !pred(obj); };
        return std::all_of(m_preds.begin(), m_preds.end(), check_pred);
    }

private:
    std::vector<pred_type> m_preds;
};

template <typename BT>
class lyciator
{
    using calc_rv_t = typename banner_traits<BT>::calc_rv;
    using calc_it_t = typename std::vector<calc_rv_t>::iterator;

    template <typename T>
    friend std::vector<T> auction(std::vector<T> banners, std::size_t lots, filter<T> banner_filter);

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
        return std::move(by_adv);
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
                max_prices.push_back(thread_pool.enqueue([&]{ return max_sum_price_block<BT>{lots}(kv.second);} ));
            }
        }

        // извлечь данные из фьючерсов
        std::vector<calc_rv_t> results;
        std::transform(max_prices.begin(), max_prices.end(), std::back_inserter(results),
               [](std::future<calc_rv_t>& f)
               {
                   return f.get();
               });

        // TODO: NRVO or std::move (entire project question)
        return std::move(results);
    }

    calc_it_t prioritize(std::vector<calc_rv_t>& results) const
    {
        // найти списки баннеров с самым высоким приоритетом
        std::sort(results.begin(), results.end(), typename calc_rv_t::greater{});
        return std::adjacent_find(results.begin(), results.end(), typename calc_rv_t::non_equal{});
    }

    calc_it_t choose_uniformly(calc_it_t left_it, calc_it_t right_it)
    {
        // равновероятно выбрать из множества равнопредпочитаемых списков баннеров
        std::uniform_int_distribution<> dis(0, std::distance(left_it, right_it));
        return std::next(left_it, dis(gen));
    }

    std::vector<BT> run_auction(std::size_t lots, filter<BT>&& banner_filter)
    {
        // процесс аукциона
        auto filtered_banner_end = make_filter(std::move(banner_filter));
        auto by_adv_id = group_by_id(filtered_banner_end);
        auto max_prices = find_best_concurrently(by_adv_id, lots);
        const auto right_it = prioritize(max_prices);
        auto best_it = choose_uniformly(max_prices.begin(), right_it);

        // переместить в возвращаемый результат
        const auto& cont = by_adv_id[best_it->id];
        auto from_it = cont.cbegin();
        auto to_it = std::next(cont.cbegin(), best_it->offset);
        std::vector<BT> rv;
        std::move(from_it, to_it, std::back_inserter(rv));
        return rv;
    }

    std::random_device rd;
    std::mt19937 gen;
    std::vector<BT> m_banners;
};

template <typename BT>
std::vector<BT> auction(std::vector<BT> banners, std::size_t lots, filter<BT> banner_filter)
{
    lyciator<BT> lyc(std::move(banners));
	return lyc.run_auction(lots, std::move(banner_filter));;
}

int main() {

    using banner_t = banner_ex;
    std::vector<banner_t> banners {
            {1, 200, 100},
            {1, 200},
            {1, 300, 100},
            {2, 400},
            {3, 500},
            {4, 600},
            {5, 700, 250}
    };


/*
    using banner_t = banner;
    std::vector<banner_t> banners {
            {1, 200},
            {2, 200},
            {3, 300},
            {4, 400},
            {5, 800},
            {6, 600},
            {7, 600},
            {8, 700},
            {9, 600},
            {2, 600},
    };
*/
    filter<banner_t> banner_filter;
	banner_filter.add([](const banner_t& b) -> bool
	{
	    return b.countries.empty() ? true : b.countries.count("russia");
	});

	const auto& res = auction(banners, 10, banner_filter);
	std::cout << res << std::endl;

	return 0;
}
