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
    using banner_t = banner;

    std::vector<banner_t> banners {
            {1, 200},
            {2, 200},
            {2, 100}
    };

    filter<banner_t> banner_filter;
	banner_filter.add([](const banner_t& b) -> bool
	{
	    return true;//b.countries.empty() ? true : b.countries.count("russia");
	});

	const auto& res = auction(banners, 2);//, banner_filter);
	std::cout << res << std::endl;

	return 0;
}
