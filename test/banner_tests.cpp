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