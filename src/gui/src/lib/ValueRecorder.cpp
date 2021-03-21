#include "ValueRecorder.h"

ValueRecorder::ValueRecorder(std::size_t size)
{
    assert((size > 2) && "Size must be 2 or greater");

    time_values.resize(size);
    state_values.resize(size);

    owner_time_values.resize(size);
    owner_values.resize(size);
}

std::size_t ValueRecorder::size() const { return state_values.size(); }

void ValueRecorder::add(uint16_t value, Scheduling::counter_type time)
{
    if (is_paused())
    {
        return;
    }

    std::copy(state_values.begin() + 1, state_values.end(), state_values.begin());
    std::copy(time_values.begin() + 1, time_values.end(), time_values.begin());

    state_values[state_values.size() - 1] = static_cast<double>(value);
    time_values[time_values.size() - 1] = static_cast<double>(time);
}

void ValueRecorder::change_owner(void* owner, Scheduling::counter_type time)
{
    if (is_paused())
    {
        return;
    }

    std::copy(owner_values.begin() + 1, owner_values.end(), owner_values.begin());
    std::copy(owner_time_values.begin() + 1, owner_time_values.end(), owner_time_values.begin());

    owner_values[owner_values.size() - 1] =
            static_cast<uint32_t>(0xffffffff & (reinterpret_cast<uint64_t>(owner)));
    owner_time_values[owner_time_values.size() - 1] = static_cast<double>(time);
}

const double* ValueRecorder::time_series() const { return time_values.data(); }
const double* ValueRecorder::data_series() const { return state_values.data(); }

const double* ValueRecorder::owner_time_series() const { return owner_time_values.data(); }
const std::uint32_t* ValueRecorder::owner_series() const { return owner_values.data(); }
