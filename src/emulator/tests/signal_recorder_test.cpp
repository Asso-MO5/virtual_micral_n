#include "SignalRecorder.h"

#include <emulation_core/src/Edge.h>

#include <gmock/gmock.h>

using namespace testing;

TEST(SignalRecorder, starts_with_a_size)
{
    const size_t SIZE = 5;
    auto clock_recorder = SignalRecorder{SIZE};

    ASSERT_THAT(clock_recorder.size(), Eq(SIZE));
}

TEST(SignalRecorder, gets_timed_values)
{
    const size_t SIZE = 6;
    auto clock_recorder = SignalRecorder{SIZE};

    clock_recorder.add(Edge{Edge::Front::RISING, Scheduling::counter_type{100}});
    clock_recorder.add(Edge{Edge::Front::FALLING, Scheduling::counter_type{200}});
    clock_recorder.add(Edge{Edge::Front::RISING, Scheduling::counter_type{300}});

    ASSERT_THAT(clock_recorder.time_series(), NotNull());
    ASSERT_THAT(clock_recorder.data_series(), NotNull());

    ASSERT_THAT(clock_recorder.time_series()[0], Le(100.f));
    ASSERT_THAT(clock_recorder.data_series()[0], Eq(0.f));

    ASSERT_THAT(clock_recorder.time_series()[1], Ge(100.f));
    ASSERT_THAT(clock_recorder.data_series()[1], Eq(1.f));

    ASSERT_THAT(clock_recorder.time_series()[2], Le(200.f));
    ASSERT_THAT(clock_recorder.data_series()[2], Eq(1.f));

    ASSERT_THAT(clock_recorder.time_series()[3], Ge(200.f));
    ASSERT_THAT(clock_recorder.data_series()[3], Eq(0.f));

    ASSERT_THAT(clock_recorder.time_series()[4], Le(300.f));
    ASSERT_THAT(clock_recorder.data_series()[4], Eq(0.f));

    ASSERT_THAT(clock_recorder.time_series()[5], Ge(300.f));
    ASSERT_THAT(clock_recorder.data_series()[5], Eq(1.f));
}

TEST(SignalRecorder, can_be_paused)
{
    const size_t SIZE = 6;
    auto clock_recorder = SignalRecorder{SIZE};

    clock_recorder.pause();
    clock_recorder.add(Edge{Edge::Front::RISING, Scheduling::counter_type{100}});
    clock_recorder.add(Edge{Edge::Front::FALLING, Scheduling::counter_type{200}});
    clock_recorder.add(Edge{Edge::Front::RISING, Scheduling::counter_type{300}});

    for (auto index = 0; index < SIZE; index += 1)
    {
        ASSERT_THAT(clock_recorder.time_series()[index], Eq(-100.0));
        ASSERT_THAT(clock_recorder.data_series()[index], Eq(0.0));
    };
}

TEST(SignalRecorder, can_be_resumed)
{
    const size_t SIZE = 6;
    auto clock_recorder = SignalRecorder{SIZE};

    clock_recorder.pause();
    clock_recorder.add(Edge{Edge::Front::RISING, Scheduling::counter_type{100}});
    clock_recorder.add(Edge{Edge::Front::FALLING, Scheduling::counter_type{200}});
    clock_recorder.add(Edge{Edge::Front::RISING, Scheduling::counter_type{300}});

    clock_recorder.resume();
    clock_recorder.add(Edge{Edge::Front::RISING, Scheduling::counter_type{400}});

    for (auto index = 0; index < SIZE - 2; index += 1)
    {
        ASSERT_THAT(clock_recorder.time_series()[index], Eq(-100.0));
        ASSERT_THAT(clock_recorder.data_series()[index], Eq(0.0));
    }

    ASSERT_THAT(clock_recorder.time_series()[SIZE - 2], Le(400.0));
    ASSERT_THAT(clock_recorder.data_series()[SIZE - 2], Eq(0.0));

    ASSERT_THAT(clock_recorder.time_series()[SIZE - 1], Ge(400.0));
    ASSERT_THAT(clock_recorder.data_series()[SIZE - 1], Eq(1.0));
}
