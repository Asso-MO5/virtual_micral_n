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
    MockDestination destination_object;
    MockSource source_object;

    auto connector = connect(source_object.source_value, this);
    connector.to(destination_object.destination_value);

    ASSERT_THAT(*source_object.source_value, Eq(0));
    ASSERT_THAT(*destination_object.destination_value, Eq(0));

    source_object.source_value.request(&source_object, Scheduling::counter_type{0});
    source_object.source_value.set(0xfe, Scheduling::counter_type{100}, &source_object);

    ASSERT_THAT(*destination_object.destination_value, Eq(0xfe));
}
