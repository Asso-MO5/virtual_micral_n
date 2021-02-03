#include "Clock.h"

Clock::Clock(Frequency frequency) : frequency(frequency) {
}

void Clock::step()
{
    auto activation_time = get_next_activation_time();

    const uint64_t period_in_ns = frequency.get_period_as_ns() / 2; // Two phases in a full cycle.

    auto before = state;
    state.invert(activation_time);

    Edge edge{before, state, activation_time};
    edge_callback(edge);

    set_next_activation_time(activation_time + period_in_ns);
}

State Clock::get_state() const { return state; }

void Clock::register_trigger(std::function<void(Edge)> callback)
{
    edge_callback = std::move(callback);
}
