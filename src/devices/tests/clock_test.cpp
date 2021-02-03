#include "Clock.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(Clock, clock_is_a_schedulable)
{
    Clock clock{400'000_hz};
    ASSERT_THAT(clock.get_next_activation_time(), Eq(0));
}

TEST(Clock, clock_steps_at_the_correct_frequency)
{
    Clock clock{400'000_hz};
    clock.step();
    ASSERT_THAT(clock.get_next_activation_time(), Eq(2500 / 2));
}

TEST(Clock, clock_starts_low)
{
    Clock clock{400'000_hz};
    ASSERT_THAT(clock.get_state(), Eq(State::LOW));
}

TEST(Clock, clock_changes_state)
{
    Clock clock{400'000_hz};
    clock.step();
    ASSERT_THAT(clock.get_state(), Eq(State::HIGH));
}

TEST(Clock, clock_triggers_on_edge)
{
    Clock clock{400'000_hz};

    Edge received_edge;
    clock.register_trigger([&received_edge](Edge edge) { received_edge = edge; });
    clock.step();
    ASSERT_THAT(received_edge, Eq(Edge::Front::RISING));

    clock.step();
    ASSERT_THAT(received_edge, Eq(Edge::Front::FALLING));

    clock.step();
    ASSERT_THAT(received_edge, Eq(Edge::Front::RISING));
}
