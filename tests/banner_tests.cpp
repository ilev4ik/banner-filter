#include <gtest/gtest.h>

#include "banner.h"
#include "banner_ex.h"

#include "lyciator.h"

namespace test_utils {
    template<typename T = banner>
    using empty_banner_list = std::vector<T>;
}

using namespace test_utils;

TEST(SimpleBannerList, EmptyBannerList)
{
    ASSERT_EQ(auction(empty_banner_list<>{}, 1), empty_banner_list<>{});
}

TEST(SimpleBannerList, ZeroLots)
{
    std::vector<banner> banners {
            {1, 200},
            {1, 200},
            {1, 300},
            {2, 400},
            {3, 500},
            {4, 600},
            {5, 700}
    };
    ASSERT_THROW(auction(banners, 0), std::invalid_argument);
}

TEST(SimpleBannerList, EmptyZeroBanner)
{
    ASSERT_THROW(auction(empty_banner_list<>{}, 0), std::invalid_argument);
}

TEST(SimpleBannerList, SingleBest)
{
    empty_banner_list<> banners {
            {1, 100},
            {2, 200}
    };
    ASSERT_EQ(auction(banners, 1),(empty_banner_list<>{{2,200}}));
}

TEST(SimpleBannerList, AccumulatedSingleLotBest)
{
    empty_banner_list<> banners {
            {1, 200},
            {2, 200},
            {2, 100}
    };
    ASSERT_EQ(auction(banners, 1),(empty_banner_list<>{{2, 200}}));
}

TEST(SimpleBannerList, AccumulatedMultipleLotsBest)
{
    empty_banner_list<> banners {
            {1, 200},
            {2, 200},
            {2, 100}
    };
    ASSERT_EQ(auction(banners, 2),(empty_banner_list<>{{2, 200}, {2, 100}}));
}

TEST(SimpleBannerList, AccumulatedMultipleLotsBest_ExtraLots)
{
    empty_banner_list<> banners {
            {1, 200},
            {2, 200},
            {2, 100}
    };
    ASSERT_EQ(auction(banners, 15),(empty_banner_list<>{{2, 200}, {2, 100}}));
}

TEST(SimpleBannerList, MultiAccumulatedMultipleLotsBest)
{
    empty_banner_list<> banners {
            {1, 200},
            {1, 100},
            {2, 200},
            {2, 100},
            {2, 1}
    };
    ASSERT_EQ(auction(banners, 3),(empty_banner_list<>{{2, 200}, {2, 100}, {2, 1}}));
}

TEST(ComplexBannerList, BasicFilterOnEmptyList)
{
    empty_banner_list<> banners;
    filter<banner>filter;
    filter.add([](const banner& b) -> bool
    {
        return b.countries.empty() ? true : b.countries.count("russia");
    });
    ASSERT_EQ(auction(banners, 10, filter), empty_banner_list<>{});
}

TEST(ComplexBannerList, BasicFilterApplied)
{
    empty_banner_list<> banners {
            {1, 100, "russia"},
            {2, 200, "usa"},
            {3, 300, "germany"}
    };

    filter<banner>filter;
    filter.add([](const banner& b) -> bool
    {
       return b.countries.empty() ? true : b.countries.count("russia");
    });
    ASSERT_EQ(auction(banners, 1, filter), (empty_banner_list<>{{1, 100, "russia"}}));
}

TEST(ComplexBannerList, BasicFilterAppliedAccumulated)
{
    empty_banner_list<> banners {
            {1, 100, "russia"},
            {1, 200, "russia"},
            {2, 200, "usa"},
            {3, 500, "usa"},
            {3, 300, "germany"}
    };

    filter<banner>filter;
    filter.add([](const banner& b) -> bool
    {
        return b.countries.empty() ? true : (b.countries.count("russia") || b.countries.count("usa"));
    });
    ASSERT_EQ(auction(banners, 3, filter), (empty_banner_list<>{{3, 500, "usa"}}));
}

TEST(ComplexBannerList, AllObjectsFiltered)
{
    empty_banner_list<> banners {
            {1, 100, "france"},
            {1, 200, "spain"},
            {2, 200, "italy"},
            {3, 500, "japan"},
            {3, 300, "canada"}
    };
    filter<banner>filter;
    filter.add([](const banner& b) -> bool
    {
        return b.countries.empty() ? true : (b.countries.count("russia") || b.countries.count("usa"));
    });
    ASSERT_EQ(auction(banners, 100, filter), (empty_banner_list<>{}));
}

TEST(InternalFeatures, IDsMoreThanHardwareConcurrency)
{
    const auto hc = std::thread::hardware_concurrency();
    const auto banners_count = hc*2;
    empty_banner_list<> banners;
    banners.reserve(banners_count);

    for (std::size_t i = 0; i < banners_count; ++i) {
        banners.emplace_back(i+1, (i+1)*100);
    }
    ASSERT_EQ(auction(banners, 2), (empty_banner_list<>{{(int)banners_count, (int)banners_count*100}}));
}

TEST(DerivedBannerPriority, CustomBannerPrioritizing)
{
    empty_banner_list<banner_ex> banners {
            {1, 100, "russia", 100},
            {1, 100, "russia", 50},
            {2, 100, "usa", 100},
            {2, 100, "usa", 51},
            {2, 100, "usa", 52},
            {2, 1000, "usa", 1000},
            {2, 2000, "austria", 0}
    };
    filter<banner_ex> filter;
    filter.add([](const banner_ex& b) -> bool {
        return b.price < 1000 && !b.countries.count("austria");
    });

    const auto expected = empty_banner_list<banner_ex>{
        {2, 100, "usa", 100},
        {2, 100, "usa", 52}
    };
    ASSERT_EQ(auction(banners, 2, filter), expected);
}
