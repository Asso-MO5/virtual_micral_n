#include "ConnectedData.h"

#include <gmock/gmock.h>

using namespace testing;

TEST(ConnectedData, can_store_data)
{
    ConnectedData data;
    data.write(0xfe);
    ASSERT_THAT(data.read(), Eq(0xfe));
}

TEST(ConnectedData, can_check_if_owning_the_bus)
{
    ConnectedData data;

    data.take_bus();
    ASSERT_TRUE(data.is_owning_bus());
    data.release_bus();
    ASSERT_FALSE(data.is_owning_bus());
}
