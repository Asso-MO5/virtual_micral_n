#include "SignalRecorder.h"

#include <emulation_core/src/Edge.h>

#include <gmock/gmock.h>

using namespace testing;

TEST(ClockRecorder, starts_with_a_size)
{
    const size_t SIZE = 5;
    auto clock_recorder = SignalRecorder{SIZE};

    ASSERT_THAT(clock_recorder.size(), Eq(SIZE));
}

TEST(ClockRecorder, gets_timed_values)
{
    const size_t SIZE = 6;
    auto clock_recorder = SignalRecorder{SIZE};

    clock_recorder.add(Edge{Edge::Front::RISING, Scheduling::counter_type{100}});
    clock_recorder.add(Edge{Edge::Front::FALLING, Scheduling::counter_type{200}});
    clock_recorder.add(Edge{Edge::Front::RISING, Scheduling::counter_type{300}});

    ASSERT_THAT(clock_recorder.time_series(), NotNull());
    ASSERT_THAT(clock_recorder.state_series(), NotNull());

    ASSERT_THAT(clock_recorder.time_series()[0], Le(100.f));
    ASSERT_THAT(clock_recorder.state_series()[0], Eq(0.f));

    ASSERT_THAT(clock_recorder.time_series()[1], Ge(100.f));
    ASSERT_THAT(clock_recorder.state_series()[1], Eq(1.f));

    ASSERT_THAT(clock_recorder.time_series()[2], Le(200.f));
    ASSERT_THAT(clock_recorder.state_series()[2], Eq(1.f));

    ASSERT_THAT(clock_recorder.time_series()[3], Ge(200.f));
    ASSERT_THAT(clock_recorder.state_series()[3], Eq(0.f));

    ASSERT_THAT(clock_recorder.time_series()[4], Le(300.f));
    ASSERT_THAT(clock_recorder.state_series()[4], Eq(0.f));

    ASSERT_THAT(clock_recorder.time_series()[5], Ge(300.f));
    ASSERT_THAT(clock_recorder.state_series()[5], Eq(1.f));
}
