#include "ValueRecorder.h"

#include <emulation_core/src/Scheduling.h>
#include <gmock/gmock.h>

using namespace testing;

TEST(ValueRecorder, starts_with_a_size)
{
    const size_t SIZE = 5;
    auto recorder = ValueRecorder{SIZE};

    ASSERT_THAT(recorder.size(), Eq(SIZE));
}

TEST(ValueRecorder, gets_timed_values)
{
    const size_t SIZE = 5;
    auto recorder = ValueRecorder{SIZE};

    recorder.add(0xffff, Scheduling::counter_type{100});

    ASSERT_THAT(recorder.time_series(), NotNull());
    ASSERT_THAT(recorder.value_series(), NotNull());

    ASSERT_THAT(recorder.time_series()[SIZE - 1], Eq(100.f));
    ASSERT_THAT(recorder.value_series()[SIZE - 1], Eq(static_cast<double>(0xffff)));
}

TEST(ValueRecorder, can_be_paused)
{
    const size_t SIZE = 5;
    auto recorder = ValueRecorder{SIZE};

    recorder.pause();
    recorder.add(0xffff, Scheduling::counter_type{100});

    for (auto index = 0; index < SIZE; index += 1)
    {
        ASSERT_THAT(recorder.time_series()[index], Eq(0.f));
        ASSERT_THAT(recorder.value_series()[index], Eq(0.f));
    }
}

TEST(ValueRecorder, records_the_owner_change)
{
    const size_t SIZE = 5;
    auto recorder = ValueRecorder{SIZE};

    recorder.change_owner(nullptr, Scheduling::counter_type{150});
    recorder.change_owner(reinterpret_cast<void*>(&recorder), Scheduling::counter_type{250});

    ASSERT_THAT(recorder.owner_time_series(), NotNull());
    ASSERT_THAT(recorder.owner_series(), NotNull());

    ASSERT_THAT(recorder.owner_time_series()[SIZE - 2], Eq(150.f));
    ASSERT_THAT(recorder.owner_series()[SIZE - 2], Eq(0));

    ASSERT_THAT(recorder.owner_time_series()[SIZE - 1], Eq(250.f));
    ASSERT_THAT(recorder.owner_series()[SIZE - 1],
                Eq(static_cast<uint32_t>(0xffffffff & (reinterpret_cast<uint64_t>(&recorder)))));
}

TEST(ValueRecorder, can_be_resumed)
{
    const size_t SIZE = 5;
    auto recorder = ValueRecorder{SIZE};

    recorder.pause();
    recorder.add(0xffff, Scheduling::counter_type{100});

    recorder.resume();
    recorder.add(0xf0f0, Scheduling::counter_type{200});

    for (auto index = 0; index < SIZE - 1; index += 1)
    {
        ASSERT_THAT(recorder.time_series()[index], Eq(0.f));
        ASSERT_THAT(recorder.value_series()[index], Eq(0.f));
    }

    ASSERT_THAT(recorder.time_series()[SIZE - 1], Eq(200.f));
    ASSERT_THAT(recorder.value_series()[SIZE - 1], Eq(static_cast<double>(0xf0f0)));
}
