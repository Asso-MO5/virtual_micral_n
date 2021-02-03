
#include "ClockRecorder.h"

#include <algorithm>
#include <cassert>

ClockRecorder::ClockRecorder(std::size_t size)
{
    assert((size > 2) && "Size must be 2 or greater");
    time_values.resize(size);
    state_values.resize(size);

    std::fill(begin(time_values), end(time_values), 0);
    std::fill(begin(state_values), end(state_values), 0);
}

std::size_t ClockRecorder::size() const { return time_values.size(); }
const float* ClockRecorder::time_series() const { return time_values.data(); }
const float* ClockRecorder::state_series() const { return state_values.data(); }

void ClockRecorder::add(Edge edge)
{
    auto time = edge.time();

    std::copy(state_values.begin() + 2, state_values.end(), state_values.begin());
    std::copy(time_values.begin() + 2, time_values.end(), time_values.begin());
    state_values[state_values.size() - 2] = (edge == Edge::Front::RISING) ? 0.f : 1.f;
    time_values[time_values.size() - 2] = time - 1;

    state_values[state_values.size() - 1] = (edge == Edge::Front::RISING) ? 1.f : 0.f;
    time_values[time_values.size() - 1] = time;
}
