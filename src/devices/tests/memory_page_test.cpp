#include "MemoryPage.h"

#include <gmock/gmock.h>

using namespace testing;

TEST(ActiveMemoryPage, references_a_data_pool_to_write)
{
    uint8_t buffer[128];
    ActiveMemoryPage page{buffer};

    page.write(0, 0xff);

    ASSERT_THAT(buffer[0], Eq(0xff));
}

TEST(ActiveMemoryPage, references_a_data_pool_to_read)
{
    uint8_t buffer[128];
    buffer[5] = 0xfe;
    ActiveMemoryPage page{buffer};

    ASSERT_THAT(page.read(5), Eq(0xfe));
}

TEST(InactiveMemoryPage, returns_zero)
{
    InactiveMemoryPage page{};

    ASSERT_THAT(page.read(5), Eq(0));
}
