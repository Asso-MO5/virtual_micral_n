#include "Averager.h"

#include <gmock/gmock.h>

using namespace testing;

TEST(Averager, with_one_value_gives_that_value)
{
    Averager<uint32_t, 16> average{};

    average.push(1234);

    ASSERT_THAT(average.average(), Eq(1234));
}

TEST(Averager, with_two_values_gives_that_average)
{
    Averager<uint32_t, 16> average{};

    average.push(100);
    average.push(200);

    ASSERT_THAT(average.average(), Eq(150));
}

TEST(Averager, with_size_two_average_the_two_most_recent_values)
{
    Averager<uint32_t, 2> average{};

    average.push(100);
    average.push(200);
    average.push(300);

    ASSERT_THAT(average.average(), Eq(250));
}

