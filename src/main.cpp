#include <iostream>

#include "banner.h"
#include "banner_ex.h"

#include "lyciator.h"

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
	os << '[';
	for (auto&& elem : vec) {
		os << elem << ", ";
	}
	os << ']';
	return os;
}

int main()
{
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

    filter<banner_t> banner_filter;
	banner_filter.add([](const banner_t& b) -> bool
	{
	    return b.countries.empty() ? true : b.countries.count("russia");
	});

	const auto& res = auction(banners, 0, banner_filter);
	std::cout << res << std::endl;

	return 0;
}
