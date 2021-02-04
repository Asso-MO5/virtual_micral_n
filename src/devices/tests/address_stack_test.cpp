#include "CPU8008.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(AddressStack, initializes_with_pc_at_0)
{
    AddressStack address_stack{};

    ASSERT_THAT(address_stack.get_pc(), Eq(0x0000));
}

TEST(AddressStack, get_provide_low_and_high_pc_address_and_increment_it)
{
    AddressStack address_stack{0x10fe};

    ASSERT_THAT(address_stack.get_low_pc_and_inc(), Eq(0xfe));
    ASSERT_THAT(address_stack.get_high_pc(), Eq(0x10));
    ASSERT_THAT(address_stack.get_low_pc_and_inc(), Eq(0xff));
    ASSERT_THAT(address_stack.get_high_pc(), Eq(0x10));
    ASSERT_THAT(address_stack.get_low_pc_and_inc(), Eq(0x00));
    ASSERT_THAT(address_stack.get_high_pc(), Eq(0x11));
}

TEST(AddressStack, get_provide_low_and_high_pc_address_without_incrementing_it)
{
    AddressStack address_stack{0x10fe};

    ASSERT_THAT(address_stack.get_low_pc_no_inc(), Eq(0xfe));
    ASSERT_THAT(address_stack.get_high_pc(), Eq(0x10));
    ASSERT_THAT(address_stack.get_low_pc_no_inc(), Eq(0xfe));
    ASSERT_THAT(address_stack.get_high_pc(), Eq(0x10));
}

TEST(AddressStack, pc_is_14_bit_wide)
{
    AddressStack address_stack{0x3fff};

    ASSERT_THAT(address_stack.get_low_pc_and_inc(), Eq(0xff));
    ASSERT_THAT(address_stack.get_high_pc(), Eq(0x3f));
    ASSERT_THAT(address_stack.get_low_pc_and_inc(), Eq(0x00));
    ASSERT_THAT(address_stack.get_high_pc(), Eq(0x00));

    AddressStack address_stack_2{0xbfff};
    ASSERT_THAT(address_stack_2.get_low_pc_and_inc(), Eq(0xff));
    ASSERT_THAT(address_stack_2.get_high_pc(), Eq(0x3f));
}

TEST(AddressStack, can_push_an_address)
{
    AddressStack address_stack{0x1000};

    address_stack.push(0x2000);

    ASSERT_THAT(address_stack.get_low_pc_and_inc(), Eq(0x00));
    ASSERT_THAT(address_stack.get_high_pc(), Eq(0x20));
}

TEST(AddressStack, can_pop_an_address)
{
    AddressStack address_stack{0x1000};

    address_stack.push(0x2000);
    address_stack.pop();

    ASSERT_THAT(address_stack.get_low_pc_and_inc(), Eq(0x00));
    ASSERT_THAT(address_stack.get_high_pc(), Eq(0x10));
}
