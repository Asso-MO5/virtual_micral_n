#include "Edge.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(Edge, can_be_created_with_a_value_implicitely)
{
    Edge edge{Edge::Front::RISING};
    ASSERT_THAT(edge, Eq(Edge::Front::RISING));
    ASSERT_THAT(edge.time(), Eq(0));
}

TEST(Edge, can_be_created_with_a_timestamp)
{
    Edge edge{Edge::Front::RISING, Scheduling::counter_type{1000}};
    ASSERT_THAT(edge, Eq(Edge::Front::RISING));
    ASSERT_THAT(edge.time(), Eq(1000));
}

TEST(Edge, can_be_deduced_from_two_states)
{
    Edge edge_1{State::LOW, State::HIGH, Scheduling::counter_type{}};
    ASSERT_THAT(edge_1, Eq(Edge::Front::RISING));

    Edge edge_2{State::HIGH, State::LOW, Scheduling::counter_type{}};
    ASSERT_THAT(edge_2, Eq(Edge::Front::FALLING));

    Edge edge_3{State::HIGH, State::HIGH, Scheduling::counter_type{}};
    ASSERT_THAT(edge_3, Eq(Edge::Front::NONE));
}

TEST(Edge, can_be_applied_to_a_state)
{
    Edge edge_1{State::LOW, State::HIGH, Scheduling::counter_type{1000}};
    State state_1 = edge_1.apply();
    ASSERT_THAT(state_1, Eq(State::HIGH));
    ASSERT_THAT(state_1.last_change(), Eq(Scheduling::counter_type{1000}));

    Edge edge_2{State::HIGH, State::LOW, Scheduling::counter_type{2000}};
    State state_2 = edge_2.apply();
    ASSERT_THAT(state_2.last_change(), Eq(Scheduling::counter_type{2000}));

}