#include "ValueRecorder.h"

ValueRecorder::ValueRecorder(std::size_t size, uint8_t bus_width, OwnerTracking track_owners)
    : data_bus_width{bus_width}
{
    assert((size > 2) && "Size must be 2 or greater");

    time_values.resize(size);
    state_values.resize(size);

    std::fill(begin(time_values), end(time_values), -100.0);
    std::fill(begin(state_values), end(state_values), 0);

    if (track_owners == TRACK_OWNERS)
    {
        owner_time_values.resize(size);
        owner_values.resize(size);

        std::fill(begin(owner_time_values), end(owner_time_values), -100.0);
        std::fill(begin(owner_values), end(owner_values), 0);
    }
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
    if (is_paused() || owner_values.empty())
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
const std::uint32_t* ValueRecorder::owner_data_series() const { return owner_values.data(); }
size_t ValueRecorder::owner_size() const { return owner_time_values.size(); }
uint8_t ValueRecorder::bus_width() const { return data_bus_width; }
