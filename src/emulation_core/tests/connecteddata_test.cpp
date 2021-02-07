#include "ConnectedData.h"

#include <gmock/gmock.h>

using namespace testing;

TEST(ConnectedData, can_store_data)
{
    ConnectedData data;
    data.write(0xfe);
    ASSERT_THAT(data.read(), Eq(0xfe));
}
