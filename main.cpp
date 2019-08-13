#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <future>
#include <random>

#include "hash_back_inserter.h"
#include "simple_thread_pool.h"

template <typename T>
struct banner_traits {};

struct banner
{
    banner(int id, int pr) : adv_id(id), price(pr) {}

	banner(int id, int pr, const std::string& country)
	    : adv_id(id), price(pr)
	{
		countries.insert(country);
	}

	int adv_id;
	int price;
	std::unordered_set<std::string> countries;

    friend std::ostream& operator<<(std::ostream& os, const banner& b) {
        return os << '{' << b.adv_id << ", " << b.price << '}';
    }
};

template <>
struct banner_traits<banner>
{
    struct greater
    {
        bool operator()(const banner& lhs, const banner& rhs) const {
            return lhs.price > rhs.price;
        }
    };

    struct equal
    {
        bool operator()(const banner& lhs, const banner& rhs) const {
            return lhs.price == rhs.price;
        }
    };

    using sum_t = std::size_t;
    struct summator
    {
        sum_t operator()(sum_t acc, const banner& b) {
            return acc + b.price;
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

        std::size_t sum;
        std::size_t offset;
        int id;
    };
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

template <>
struct banner_traits<banner_ex>
{
    using sum_t = typename banner_ex::rank_t;

    struct greater
    {
        bool operator()(const banner_ex& lhs, const banner_ex& rhs) const {
            // lexicographical_compare of tuples
            return lhs.rank() > rhs.rank();
        }
    };

    struct summator
    {
        sum_t operator()(sum_t acc, const banner_ex& b) {
            auto r = b.rank();
            std::get<0>(acc) += std::get<0>(r);
            std::get<1>(acc) += std::get<1>(r);
            std::get<2>(acc) += std::get<2>(r);
            std::get<3>(acc) += std::get<3>(r);
            return acc;
        }
    };
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
std::vector<BT> auction(std::vector<BT> banners, std::size_t lots, filter<BT> banner_filter)
{
    using calc_rv_t = typename banner_traits<BT>::calc_rv;
    // отфильтровать
    auto filtered_banner_end = std::remove_if(banners.begin(), banners.end(), std::move(banner_filter));

    // разбить на группы по adv_id
	std::unordered_map<int, std::vector<BT>> by_adv;
	std::move(banners.begin(), filtered_banner_end, hash_back_inserter(by_adv));
	if (by_adv.empty()) return {};

    std::vector<std::future<calc_rv_t>> max_prices;
    {
        // отправить на выделенные потоки задачи на подсчёт суммы цен (решения сути задачи)
        simple_thread_pool thread_pool(std::thread::hardware_concurrency());

        for (auto &&kv : by_adv) {
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

	// найти списки баннеров с одинаковой самой высокой ценой
	std::sort(results.begin(), results.end(), typename calc_rv_t::greater{});
	auto right_it = std::adjacent_find(results.begin(), results.end(), typename calc_rv_t::non_equal{});

	// равновероятно выбрать из этого списка 1
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, std::distance(results.begin(), right_it));
    auto max_it = std::next(results.begin(), dis(gen));

    // переместить в возвращаемый результат
    const auto& cont = by_adv[max_it->id];
    auto from_it = cont.begin();
    auto to_it = std::next(cont.begin(), max_it->offset);
    std::vector<BT> v;
    std::move(from_it, to_it, std::back_inserter(v));

	return v;
}

int main() {

/*
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
*/

    using banner_t = banner;
    std::vector<banner_t> banners {
            {1, 200},
            {1, 200},
            {1, 300},
            {2, 400},
            {3, 500},
            {4, 600},
            {5, 700}
    };

    filter<banner_t> banner_filter;
	banner_filter.add([](const banner_t& b) -> bool
	{
	    return b.countries.empty() ? true : b.countries.count("russia");
	});

	const auto& res = auction(banners, 10, banner_filter);
	std::cout << res << std::endl;

	//std::cin.get();
	return 0;
}
