#include "ToHex.h"

#include "gmock/gmock.h"

#include <cstdint>

using namespace testing;

TEST(ToHex, converts_16_bit_integers_with_padding)
{
    const std::uint16_t VALUE = 0x1234;
    auto converted = utils::to_hex<std::uint16_t, 4>(VALUE);

    ASSERT_THAT(converted, Eq("$1234"));
}

TEST(ToHex, converts_16_bit_integers_with_runtime_padding)
{
    const std::uint16_t VALUE = 0x1234;
    auto converted = utils::to_hex<std::uint16_t>(VALUE, 4);

    ASSERT_THAT(converted, Eq("$1234"));
}

TEST(ToHex, converts_8_bit_unsigned_integers_with_runtime_padding)
{
    const auto VALUE = static_cast<std::uint8_t>(0xff);
    auto converted = utils::to_hex<std::uint8_t>(VALUE, 2);

    ASSERT_THAT(converted, Eq("$ff"));
}
