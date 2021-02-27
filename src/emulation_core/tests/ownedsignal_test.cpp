#include "OwnedSignal.h"

#include <Edge.h>
#include <gmock/gmock.h>

using namespace testing;

TEST(OwnedSignal, is_initialized_low_and_at_unscheduled_time)
{
    OwnedSignal signal;
    ASSERT_THAT(signal.get_state(), Eq(State::LOW));
    ASSERT_THAT(signal.get_latest_change_time(), Eq(Scheduling::unscheduled()));
}

TEST(OwnedSignal, cannot_set_state_if_not_owned)
{
    OwnedSignal signal;
    uint16_t owner;

    ASSERT_THROW(
            signal.set(State::HIGH, Scheduling::counter_type{1000}, static_cast<void*>(&owner)),
            signal_error);
}

TEST(OwnedSignal, cant_set_state_if_not_owned)
{
    OwnedSignal signal;
    uint16_t owner;

    signal.request(static_cast<void*>(&owner));
    signal.set(State::HIGH, Scheduling::counter_type{1000}, static_cast<void*>(&owner));

    ASSERT_THAT(signal.get_state(), Eq(State::HIGH));
    ASSERT_THAT(signal.get_latest_change_time(), Eq(Scheduling::counter_type{1000}));
}

TEST(OwnedSignal, cannot_be_requested_if_taken)
{
    OwnedSignal signal;
    uint16_t owner;
    uint16_t other_owner;

    signal.request(static_cast<void*>(&owner));

    ASSERT_THROW(signal.request(static_cast<void*>(&other_owner)), signal_error);
}

TEST(OwnedSignal, can_be_subscribed_to)
{
    OwnedSignal signal;
    uint16_t owner;
    signal.request(static_cast<void*>(&owner));

    Edge received_edge{};

    //std::function<void(Edge)> callback
    signal.subscribe([&received_edge](Edge edge) { received_edge = edge; });

    signal.request(static_cast<void*>(&owner));
    signal.set(State::HIGH, Scheduling::counter_type{2000}, static_cast<void*>(&owner));

    ASSERT_THAT(received_edge.apply(), Eq(State::HIGH));
    ASSERT_THAT(received_edge.time(), Eq(Scheduling::counter_type{2000}));
}
