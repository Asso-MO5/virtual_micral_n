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
