#include "Clock.h"

Clock::Clock(Frequency frequency) : frequency(frequency) { phase.request(this); }

void Clock::step()
{
    auto activation_time = get_next_activation_time();

    const uint64_t period_in_ns = frequency.get_period_as_ns() / 2; // Two phases in a full cycle.

    auto state = phase.get_state();
    state.invert(activation_time);

    phase.set(state, activation_time, this);

    set_next_activation_time(activation_time + period_in_ns);
}

State Clock::get_state() const { return phase.get_state(); }

std::vector<std::shared_ptr<Schedulable>> Clock::get_sub_schedulables()
{
    return {};
}
