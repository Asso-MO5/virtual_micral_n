#include "SignalConnect.h"

#include "OwnedSignal.h"

#include <gmock/gmock.h>

using namespace testing;

namespace SignalMocks
{
    class MockSource
    {
    public:
        OwnedSignal source_signal;
    };

    class MockDestination
    {
    public:
        OwnedSignal destination_signal;
    };
}

TEST(Connect, connects_a_source_and_destination_signals)
{
    using namespace SignalMocks;
    MockSource source_object;
    MockDestination destination_object;

    auto connector = connect(source_object.source_signal, this);
    connector.to(destination_object.destination_signal);

    ASSERT_THAT(*source_object.source_signal, Eq(State::LOW));
    ASSERT_THAT(*destination_object.destination_signal, Eq(State::LOW));

    source_object.source_signal.request(&source_object);
    source_object.source_signal.set(State::HIGH, Scheduling::counter_type{100}, &source_object);

    ASSERT_THAT(*destination_object.destination_signal, Eq(State::HIGH));
}

TEST(Connect, connects_a_source_and_multiple_destination_signals)
{
    using namespace SignalMocks;
    MockSource source_object;
    MockDestination destination_object_1;
    MockDestination destination_object_2;

    connect(source_object.source_signal, this)
            .to(destination_object_1.destination_signal)
            .to(destination_object_2.destination_signal);

    ASSERT_THAT(*destination_object_1.destination_signal, Eq(State::LOW));
    ASSERT_THAT(*destination_object_2.destination_signal, Eq(State::LOW));

    source_object.source_signal.request(&source_object);
    source_object.source_signal.set(State::HIGH, Scheduling::counter_type{100}, &source_object);

    ASSERT_THAT(*destination_object_1.destination_signal, Eq(State::HIGH));
    ASSERT_THAT(*destination_object_2.destination_signal, Eq(State::HIGH));
}
