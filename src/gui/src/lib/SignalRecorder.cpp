
#include "SignalRecorder.h"

#include <algorithm>
#include <cassert>

SignalRecorder::SignalRecorder(std::size_t size)
{
    assert((size > 2) && "Size must be 2 or greater");
    time_values.resize(size);
    state_values.resize(size);

    std::fill(begin(time_values), end(time_values), 0);
    std::fill(begin(state_values), end(state_values), 0);
}

std::size_t SignalRecorder::size() const { return time_values.size(); }
const double* SignalRecorder::time_series() const { return time_values.data(); }
const double* SignalRecorder::data_series() const { return state_values.data(); }

void SignalRecorder::add(Edge edge)
{
    if (is_paused())
    {
        return;
    }

    auto time = edge.time();

    std::copy(state_values.begin() + 2, state_values.end(), state_values.begin());
    std::copy(time_values.begin() + 2, time_values.end(), time_values.begin());
    state_values[state_values.size() - 2] = (is_rising(edge)) ? 0. : 1.;
    time_values[time_values.size() - 2] = time;

    state_values[state_values.size() - 1] = (is_rising(edge)) ? 1. : 0.;
    time_values[time_values.size() - 1] = time + 1;
}

size_t SignalRecorder::owner_size() const { return 0; }
const double* SignalRecorder::owner_time_series() const { return nullptr; }
const std::uint32_t* SignalRecorder::owner_data_series() const { return nullptr; }
uint8_t SignalRecorder::bus_width() const { return 1; }
