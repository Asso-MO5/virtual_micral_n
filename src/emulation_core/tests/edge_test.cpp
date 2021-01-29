#include "emulation_core/src/Edge.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(Edge, can_be_created_with_a_value_implicitely)
{
    Edge edge{Edge::RISING};
    ASSERT_THAT(edge, Eq(Edge::RISING));
}

TEST(Edge, can_be_deduced_from_two_states)
{
    Edge edge_1{State::LOW, State::HIGH};
    ASSERT_THAT(edge_1, Eq(Edge::RISING));

    Edge edge_2{State::HIGH, State::LOW};
    ASSERT_THAT(edge_2, Eq(Edge::FALLING));

    Edge edge_3{State::HIGH, State::HIGH};
    ASSERT_THAT(edge_3, Eq(Edge::NONE));
}
