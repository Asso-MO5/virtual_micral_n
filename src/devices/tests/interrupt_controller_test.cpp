#include "InterruptController.h"

#include "Constants8008.h"

#include "gmock/gmock.h"

using namespace testing;
using namespace Constants8008;

TEST(InterruptController, needs_a_cpu_state)
{
    CpuState cpu_state{CpuState::T3};
    InterruptController controller{&cpu_state};
}

struct InterruptControllerFixture : public ::testing::Test
{
    InterruptControllerFixture()
    {
        auto& mock_edge = called_with_edge;
        controller.register_interrupt_trigger([&mock_edge](Edge edge) { mock_edge = edge; });
    }

    Edge called_with_edge{};
    CpuState cpu_state{CpuState::T3};
    InterruptController controller{&cpu_state};
};

TEST_F(InterruptControllerFixture, doesnt_call_the_callback_when_registered)
{
    ASSERT_THAT(called_with_edge.time(), Eq(0));
}

TEST_F(InterruptControllerFixture, doesnt_triggers_interrupt_immediatly)
{
    controller.wants_interrupt(Edge{Edge::Front::RISING, 10});

    ASSERT_THAT(called_with_edge.time(), Eq(0));
}

TEST_F(InterruptControllerFixture, doesnt_triggers_if_not_requested)
{
    controller.signal_phase_1(Edge{Edge::Front::RISING, 30});

    ASSERT_THAT(called_with_edge.time(), Eq(0));
}

TEST_F(InterruptControllerFixture, emits_the_interrupt_when_receiving_lead_phase_1)
{
    controller.wants_interrupt(Edge{Edge::Front::RISING, 10});
    controller.wants_interrupt(Edge{Edge::Front::FALLING, 15});

    controller.signal_phase_1(Edge{Edge::Front::RISING, 30});

    ASSERT_THAT(called_with_edge.time(), Eq(30));
    ASSERT_THAT(called_with_edge.apply(), Eq(State{State::HIGH}));
}

TEST_F(InterruptControllerFixture, keeps_interrupt_up_in_next_cycles)
{
    controller.wants_interrupt(Edge{Edge::Front::RISING, 10});
    controller.wants_interrupt(Edge{Edge::Front::FALLING, 15});

    controller.signal_phase_1(Edge{Edge::Front::RISING, 30});
    controller.signal_phase_1(Edge{Edge::Front::FALLING, 40});
    controller.signal_phase_1(Edge{Edge::Front::RISING, 100}); // Time passes
    controller.signal_phase_1(Edge{Edge::Front::FALLING, 150});
    controller.signal_phase_1(Edge{Edge::Front::RISING, 200});
    controller.signal_phase_1(Edge{Edge::Front::FALLING, 250});

    cpu_state = Constants8008::CpuState::T4;

    controller.signal_phase_1(Edge{Edge::Front::RISING, 300}); // Time passes
    controller.signal_phase_1(Edge{Edge::Front::FALLING, 350});
    controller.signal_phase_1(Edge{Edge::Front::RISING, 400});
    controller.signal_phase_1(Edge{Edge::Front::FALLING, 450});

    ASSERT_THAT(called_with_edge.time(), Eq(30));
    ASSERT_THAT(called_with_edge.apply(), Eq(State{State::HIGH}));
}

TEST_F(InterruptControllerFixture, stops_the_interrupt_when_TI1_on_falling_phase_1)
{
    controller.wants_interrupt(Edge{Edge::Front::RISING, 10});
    controller.wants_interrupt(Edge{Edge::Front::FALLING, 15});

    controller.signal_phase_1(Edge{Edge::Front::RISING, 30});
    controller.signal_phase_1(Edge{Edge::Front::FALLING, 40});
    controller.signal_phase_1(Edge{Edge::Front::RISING, 100}); // Time passes
    controller.signal_phase_1(Edge{Edge::Front::FALLING, 150});
    controller.signal_phase_1(Edge{Edge::Front::RISING, 200});
    controller.signal_phase_1(Edge{Edge::Front::FALLING, 250});

    cpu_state = Constants8008::CpuState::T1I;

    controller.signal_phase_1(Edge{Edge::Front::RISING, 300}); // Time passes
    controller.signal_phase_1(Edge{Edge::Front::FALLING, 350});

    // Last interrupt didn't change
    ASSERT_THAT(called_with_edge.time(), Eq(350));
    ASSERT_THAT(called_with_edge.apply(), Eq(State{State::LOW}));
}
