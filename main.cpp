#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>
#include <numeric>
#include <functional>
#include <future>
#include <random>

struct banner
{
	banner(int id, int pr, const std::string& country)
	    : adv_id(id), price(pr)
	{
		countries.insert(country);
	}

	int adv_id;
	int price;
	std::unordered_set<std::string> countries;
};


template <typename C>
class hash_back_insert_iterator {
protected:
    C* container;
public:
    typedef C                   container_type;
    typedef std::output_iterator_tag iterator_category;
    typedef void                value_type;
    typedef void                difference_type;
    typedef void                pointer;
    typedef void                reference;
    using mapped_type = typename C::value_type::second_type::value_type;

    explicit hash_back_insert_iterator(C& __x): container(&__x) {}

    hash_back_insert_iterator<C>& operator=(const mapped_type& val) {
        (*container)[val.adv_id].push_back(val);
        return *this;
    }

    hash_back_insert_iterator<C>& operator*() {
        return *this;
    }

    hash_back_insert_iterator<C>& operator++() {
        return *this;
    }

    hash_back_insert_iterator<C>& operator++(int) {
        return *this;
    }
};

template <class Container>
hash_back_insert_iterator<Container> hash_back_inserter(Container& c)
{
    return hash_back_insert_iterator<Container>(c);
}

std::ostream& operator<<(std::ostream& os, const banner& b) {
	return os << '{' << b.adv_id << ", " << b.price << '}';
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
	os << '[';
	for (auto&& elem : vec) {
		os << elem << ", ";
	}
	os << ']';
	return os;
}

struct calc_rv
{
    struct greater_sum
    {
        bool operator()(const calc_rv& lhs, const calc_rv& rhs) {
            return lhs.sum > rhs.sum;
        }
    };

    struct non_equal_sum
    {
        bool operator()(const calc_rv& lhs, const calc_rv& rhs) {
            return lhs.sum != rhs.sum;
        }
    };

    std::size_t sum;
    std::size_t offset;
    int id;
};

struct max_sum_price_block
{
	struct price_summator
	{
		int operator()(const banner& b, int acc) {
			return acc + b.price;
		}

		int operator()(int acc, const banner& b) {
			return acc + b.price;
		}
	};

	struct banner_price_greater
	{
		bool operator()(const banner& lhs, const banner& rhs) const {
			return lhs.price > rhs.price;;
		}
	};

	explicit max_sum_price_block(std::size_t lots): max_sum_price_block(lots, delegate_tag{})
	{
	    if (m_lots < 1) throw std::logic_error("lots count should at least one");
	}

	void operator()(std::vector<banner>& banners, std::promise<calc_rv> p) {
        // precondition: banners is not empty
        const auto pivot_offset = std::min(m_lots, banners.size());
        auto to_it = std::next(banners.begin(), pivot_offset);
        std::partial_sort(banners.begin(), to_it, banners.end(), banner_price_greater{});
        const auto max_bounded_price = std::accumulate(banners.begin(), to_it, (std::size_t) 0, price_summator{});
        p.set_value_at_thread_exit({max_bounded_price, pivot_offset, banners[0].adv_id});
	}

private:
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
        auto check_pred = [obj](const pred_type& pred) { return !pred(obj); };
        return std::all_of(m_preds.begin(), m_preds.end(), check_pred);
    }

private:
    std::vector<pred_type> m_preds;
};

std::vector<banner> auction(std::vector<banner> banners, std::size_t lots, filter<banner> banner_filter)
{
    auto filtered_banner_end = std::remove_if(banners.begin(), banners.end(), std::move(banner_filter));

	std::unordered_map<int, std::vector<banner>> by_adv;
	std::move(banners.begin(), filtered_banner_end, hash_back_inserter(by_adv));
	if (by_adv.empty()) return {};

	// hardware_concurrency
	std::vector<std::future<calc_rv>> max_prices;

	for (auto&& kv : by_adv) {
		std::promise<calc_rv> p;
		max_prices.push_back(p.get_future());
        std::thread(max_sum_price_block(lots), std::ref(kv.second), std::move(p)).detach();
	}

	std::vector<calc_rv> results;
	std::transform(max_prices.begin(), max_prices.end(), std::back_inserter(results),
		[](std::future<calc_rv>& f)
	{
		return f.get();
	});

	std::sort(results.begin(), results.end(), calc_rv::greater_sum{});

	auto right_it = std::adjacent_find(results.begin(), results.end(), calc_rv::non_equal_sum{});
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, std::distance(results.begin(), right_it));
    auto max_it = std::next(results.begin(), dis(gen));

    const auto& cont = by_adv[max_it->id];
    auto from_it = cont.begin();
    auto to_it = std::next(cont.begin(), max_it->offset);

    std::vector<banner> v;
    std::move(from_it, to_it, std::back_inserter(v));

	return v;
}

int main() {
    std::vector<banner> banners {
            {1, 200, "russia"},
            {1, 200, "russia"},
            {1, 300, "russia"},
            {2, 400, "russia"},
            {3, 500, "russia"},
            {4, 600, "russia"},
            {5, 700, "russia"}
    };

    filter<banner> banner_filter;
	banner_filter.add([](const banner& b) -> bool { return b.countries.count("russia"); });

	const auto& res = auction(banners, 3, banner_filter);
	std::cout << res << std::endl;

	std::cin.get();
	return 0;
}
