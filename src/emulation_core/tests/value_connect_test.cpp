#include "ValueConnect.h"

#include "OwnedValue.h"

#include <gmock/gmock.h>

using namespace testing;

namespace ValueMocks
{
    class MockSource
    {
    public:
        OwnedValue<uint8_t> source_value;
        OwnedValue<uint16_t> source_value_16;
    };

    class MockDestination
    {
    public:
        OwnedValue<uint8_t> destination_value;
    };

}

TEST(Connect, connects_a_source_and_destination_values)
{
    using namespace ValueMocks;
    MockSource source_object;
    MockDestination destination_object;

    auto connector = connect(source_object.source_value, this);
    connector.to(destination_object.destination_value);

    ASSERT_THAT(*source_object.source_value, Eq(0));
    ASSERT_THAT(*destination_object.destination_value, Eq(0));

    source_object.source_value.request(&source_object, Scheduling::counter_type{0});
    source_object.source_value.set(0xfe, Scheduling::counter_type{100}, &source_object);

    ASSERT_THAT(*destination_object.destination_value, Eq(0xfe));
}

TEST(Connect, connects_a_source_and_destination_values_with_a_low_mask)
{
    using namespace ValueMocks;
    MockDestination destination_object;
    MockSource source_object;

    connect(source_object.source_value_16, this)
            .mask<uint8_t>(0x00ff, 0)
            .to(destination_object.destination_value);

    ASSERT_THAT(*source_object.source_value_16, Eq(0));
    ASSERT_THAT(*destination_object.destination_value, Eq(0));

    source_object.source_value_16.request(&source_object, Scheduling::counter_type{0});
    source_object.source_value_16.set(0xfe, Scheduling::counter_type{100}, &source_object);

    ASSERT_THAT(*destination_object.destination_value, Eq(0xfe));
}

TEST(Connect, connects_a_source_and_destination_values_with_a_high_mask)
{
    using namespace ValueMocks;
    MockDestination destination_object;
    MockSource source_object;

    connect(source_object.source_value_16, this)
            .mask<uint8_t>(0xff00, 8)
            .to(destination_object.destination_value);

    ASSERT_THAT(*source_object.source_value_16, Eq(0));
    ASSERT_THAT(*destination_object.destination_value, Eq(0));

    source_object.source_value_16.request(&source_object, Scheduling::counter_type{0});
    source_object.source_value_16.set(0xfffe, Scheduling::counter_type{100}, &source_object);

    ASSERT_THAT(*destination_object.destination_value, Eq(0xff));
}
