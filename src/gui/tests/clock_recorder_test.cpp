#include "ClockRecorder.h"

#include <emulation_core/src/Edge.h>

#include <gmock/gmock.h>

using namespace testing;

TEST(ClockRecorder, starts_with_a_size)
{
    const size_t SIZE = 5;
    auto clock_recorder = ClockRecorder{SIZE};

    ASSERT_THAT(clock_recorder.size(), Eq(SIZE));
}

TEST(ClockRecorder, gets_timed_values)
{
    const size_t SIZE = 6;
    auto clock_recorder = ClockRecorder{SIZE};

    clock_recorder.add(100.f, Edge::Front::RISING);
    clock_recorder.add(200.f, Edge::Front::FALLING);
    clock_recorder.add(300.f, Edge::Front::RISING);

    ASSERT_THAT(clock_recorder.time_series(), NotNull());
    ASSERT_THAT(clock_recorder.state_series(), NotNull());

    ASSERT_THAT(clock_recorder.time_series()[0], Le(100.f));
    ASSERT_THAT(clock_recorder.state_series()[0], Eq(0.f));

    ASSERT_THAT(clock_recorder.time_series()[1], Eq(100.f));
    ASSERT_THAT(clock_recorder.state_series()[1], Eq(1.f));

    ASSERT_THAT(clock_recorder.time_series()[2], Le(200.f));
    ASSERT_THAT(clock_recorder.state_series()[2], Eq(1.f));

    ASSERT_THAT(clock_recorder.time_series()[3], Eq(200.f));
    ASSERT_THAT(clock_recorder.state_series()[3], Eq(0.f));

    ASSERT_THAT(clock_recorder.time_series()[4], Le(300.f));
    ASSERT_THAT(clock_recorder.state_series()[4], Eq(0.f));

    ASSERT_THAT(clock_recorder.time_series()[5], Eq(300.f));
    ASSERT_THAT(clock_recorder.state_series()[5], Eq(1.f));

}