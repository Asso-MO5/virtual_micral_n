#include "OwnedSignal.h"
#include "ScheduledSignal.h"

#include <gmock/gmock.h>

using namespace testing;

TEST(ScheduledSignal, takes_ownership_of_an_associated_owned_signal)
{
    OwnedSignal signal;
    ScheduledSignal scheduled_signal{signal};

    uint16_t owner;
    ASSERT_THROW(signal.request(static_cast<void*>(&owner)), signal_error);
}

TEST(ScheduledSignal, releases_ownership_when_destroyed)
{
    OwnedSignal signal;
    {
        ScheduledSignal scheduled_signal{signal};
    }

    uint16_t owner;
    signal.request(static_cast<void*>(&owner));
    signal.set(State::HIGH, Scheduling::counter_type{1000}, static_cast<void*>(&owner));

    ASSERT_THAT(signal.get_state(), Eq(State::HIGH));
}

TEST(ScheduledSignal, asserts_if_stepped_when_inactive)
{
    OwnedSignal signal;
    ScheduledSignal scheduled_signal{signal};

    ASSERT_THAT(scheduled_signal.get_next_activation_time(), Eq(Scheduling::unscheduled()));
}

TEST(ScheduledSignal, applies_to_signal_with_a_delay)
{
    const Scheduling::counter_type DELAY{200};
    const Scheduling::counter_type DURATION{1000};

    OwnedSignal signal;
    ScheduledSignal scheduled_signal{signal};

    scheduled_signal.launch(DELAY, DURATION, [](Scheduling::schedulable_id) {});
    scheduled_signal.step();

    ASSERT_THAT(signal.get_state(), Eq(State::HIGH));
    ASSERT_THAT(signal.get_latest_change_time(), Eq(DELAY));
}

TEST(ScheduledSignal, releases_signal_after_the_duration)
{
    const Scheduling::counter_type DELAY{200};
    const Scheduling::counter_type DURATION{1000};

    OwnedSignal signal;
    ScheduledSignal scheduled_signal{signal};

    scheduled_signal.launch(DELAY, DURATION, [](Scheduling::schedulable_id) {});
    scheduled_signal.step();
    scheduled_signal.step();

    ASSERT_THAT(signal.get_state(), Eq(State::LOW));
    ASSERT_THAT(signal.get_latest_change_time(), Eq(DELAY + DURATION));
    ASSERT_THAT(scheduled_signal.get_next_activation_time(), Eq(Scheduling::unscheduled()));
}
