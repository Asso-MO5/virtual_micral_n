#include "DoubleClock.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(DoubleClock, clock_is_a_schedulable)
{
    DoubleClock clock{500'000_hz};
    ASSERT_THAT(clock.get_next_activation_time(), Eq(0));
}

TEST(DoubleClock, has_a_phase_1_phase_2_cycle)
{
    DoubleClock clock{500'000_hz};

    ASSERT_THAT(clock.get_phase_1_state(), Eq(::State::LOW));
    ASSERT_THAT(clock.get_phase_2_state(), Eq(::State::LOW));
    clock.step();
    ASSERT_THAT(clock.get_phase_1_state(), Eq(::State::HIGH));
    ASSERT_THAT(clock.get_phase_2_state(), Eq(::State::LOW));
    clock.step();
    ASSERT_THAT(clock.get_phase_1_state(), Eq(::State::LOW));
    ASSERT_THAT(clock.get_phase_2_state(), Eq(::State::LOW));
    clock.step();
    ASSERT_THAT(clock.get_phase_1_state(), Eq(::State::LOW));
    ASSERT_THAT(clock.get_phase_2_state(), Eq(::State::HIGH));
    clock.step();
    ASSERT_THAT(clock.get_phase_1_state(), Eq(::State::LOW));
    ASSERT_THAT(clock.get_phase_2_state(), Eq(::State::LOW));
}

struct EdgeTester
{
    Edge received_phase_1_edge{Edge::Front::NONE};
    Edge received_phase_2_edge{Edge::Front::NONE};

    void reset()
    {
        received_phase_1_edge = Edge::Front::NONE;
        received_phase_2_edge = Edge::Front::NONE;
    }

    void test(Edge edge_1, Scheduling::counter_type time_1, Edge edge_2,
              Scheduling::counter_type time_2) const
    {
        ASSERT_THAT(received_phase_1_edge, Eq(edge_1));
        ASSERT_THAT(received_phase_1_edge.time(), Eq(time_1));
        ASSERT_THAT(received_phase_2_edge, Eq(edge_2));
        ASSERT_THAT(received_phase_2_edge.time(), Eq(time_2));
    }
};

TEST(DoubleClock, signals_edges_with_timings)
{
    DoubleClock clock{500'000_hz};

    EdgeTester tester;

    clock.phase_1.subscribe([&tester](Edge edge) { tester.received_phase_1_edge = edge; });
    clock.phase_2.subscribe([&tester](Edge edge) { tester.received_phase_2_edge = edge; });

    clock.step();
    tester.test(Edge::Front::RISING, 0, Edge::Front::NONE, 0);
    tester.reset();

    clock.step();
    tester.test(Edge::Front::FALLING, 700, Edge::Front::NONE, 0);
    tester.reset();

    clock.step();
    tester.test(Edge::Front::NONE, 0, Edge::Front::RISING, 900);
    tester.reset();

    clock.step();
    tester.test(Edge::Front::NONE, 0, Edge::Front::FALLING, 1450);
    tester.reset();

    clock.step();
    tester.test(Edge::Front::RISING, 2000, Edge::Front::NONE, 0);
}