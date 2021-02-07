#include "ConnectedData.h"
#include "DataBus.h"

#include <gmock/gmock.h>

using namespace testing;

TEST(DataBus, is_initialized_with_zero)
{
    DataBus data_bus;
    ASSERT_THAT(data_bus.read(), Eq(0));
}

TEST(DataBus, gets_the_data_applied_on_connected_data)
{
    ConnectedData data;
    auto data_bus = std::make_shared<DataBus>();

    data.connect(data_bus);
    data.take_bus();
    data.write(0x80);
    data.release_bus();

    ASSERT_THAT(data_bus->read(), Eq(0x80));
}

TEST(DataBus, transmits_the_data_applied_on_connected_data)
{
    ConnectedData data_from;
    ConnectedData data_to;

    auto data_bus = std::make_shared<DataBus>();

    data_from.connect(data_bus);
    data_to.connect(data_bus);

    data_from.take_bus();
    data_from.write(0x80);
    data_from.release_bus();

    ASSERT_THAT(data_to.read(), Eq(0x80));
}

TEST(DataBus, must_be_free_to_be_taken)
{
    ConnectedData data_from;
    ConnectedData data_to;

    auto data_bus = std::make_shared<DataBus>();

    data_from.connect(data_bus);
    data_to.connect(data_bus);

    data_from.take_bus();
    ASSERT_THROW(data_to.take_bus(), bus_error);
}

TEST(DataBus, must_be_taken_to_be_released)
{
    ConnectedData data_from;
    ConnectedData data_to;

    auto data_bus = std::make_shared<DataBus>();

    data_from.connect(data_bus);
    data_to.connect(data_bus);

    data_from.take_bus();
    ASSERT_THROW(data_to.release_bus(), bus_error);
}
