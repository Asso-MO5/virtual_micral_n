#include "State.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(State, is_time_stamped_when_assigned)
{
    State state{State::LOW};

    ASSERT_THAT(state, Eq(State::LOW));
    ASSERT_THAT(state.last_change(), Eq(0));
}

TEST(State, can_be_inverted)
{
    State state{State::LOW, Scheduling::counter_type{}};

    state.invert(Scheduling::counter_type{1000});

    ASSERT_THAT(state, State::HIGH);
    ASSERT_THAT(state.last_change(), Eq(1000));
}

TEST(State, supports_and_1)
{
    State state_1{State::LOW, Scheduling::counter_type{}};
    State state_2{State::HIGH, Scheduling::counter_type{20}};

    State result = state_1 && state_2;

    ASSERT_THAT(result, State::LOW);
    ASSERT_THAT(result.last_change(), Eq(20));
}

TEST(State, supports_and_2)
{
    State state_1{State::HIGH, Scheduling::counter_type{40}};
    State state_2{State::HIGH, Scheduling::counter_type{20}};

    State result = state_1 && state_2;

    ASSERT_THAT(result, State::HIGH);
    ASSERT_THAT(result.last_change(), Eq(40));
}

TEST(State, predicate_functions)
{
    State state_low{State::LOW, Scheduling::counter_type{20}};
    State state_high{State::HIGH, Scheduling::counter_type{40}};

    ASSERT_TRUE(is_high(state_high));
    ASSERT_FALSE(is_low(state_high));

    ASSERT_FALSE(is_high(state_low));
    ASSERT_TRUE(is_low(state_low));
}
