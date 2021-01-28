#include "Clock.h"

Clock::Clock(Frequency frequency) : frequency(frequency) {}

void Clock::step()
{
    const uint64_t period_in_ns = frequency.get_period_as_ns();
    state.invert();
    next_activation_time += period_in_ns;
}

Scheduling::counter_type Clock::get_next_activation_time() const { return next_activation_time; }

State Clock::get_state() const { return state; }
