#include "DoubleClock.h"

#include <utility>

namespace Timings
{
    const Scheduling::counter_type PULSE_WIDTH_PHASE_1 = 700;
    const Scheduling::counter_type PULSE_WIDTH_PHASE_2 = 550;
    const Scheduling::counter_type DELAY_FROM_PHASE_1_TO_2 = 200;
} // namespace Timings

DoubleClock::DoubleClock(Frequency frequency) : main_frequency(frequency) {}

void DoubleClock::step()
{
    switch (phase_count)
    {
        case 0:
            phase_1 = State::HIGH;
            edge_1_callback(Edge::RISING, next_activation_time);
            next_phase_1 = next_activation_time + main_frequency.get_period_as_ns();
            next_activation_time += Timings::PULSE_WIDTH_PHASE_1;
            break;
        case 1:
            phase_1 = State::LOW;
            edge_1_callback(Edge::FALLING, next_activation_time);
            next_activation_time += Timings::DELAY_FROM_PHASE_1_TO_2;
            break;
        case 2:
            phase_2 = State::HIGH;
            edge_2_callback(Edge::RISING, next_activation_time);
            next_activation_time += Timings::PULSE_WIDTH_PHASE_2;
            break;
        case 3:
            phase_2 = State::LOW;
            edge_2_callback(Edge::FALLING, next_activation_time);
            next_activation_time = next_phase_1;
            break;
    }
    phase_count = (phase_count + 1) % 4;
}

Scheduling::counter_type DoubleClock::get_next_activation_time() const
{
    return next_activation_time;
}

State DoubleClock::get_phase_1_state() const { return phase_1; }

State DoubleClock::get_phase_2_state() const { return phase_2; }

void DoubleClock::register_phase_1_trigger(
        std::function<void(Edge, Scheduling::counter_type)> callback)
{
    edge_1_callback = std::move(callback);
}
void DoubleClock::register_phase_2_trigger(
        std::function<void(Edge, Scheduling::counter_type)> callback)
{
    edge_2_callback = std::move(callback);
}
