#include "DoubleClock.h"

#include <utility>

DoubleClock::DoubleClock(Frequency frequency) : main_frequency(frequency) {}

void DoubleClock::step()
{
    switch (phase_count)
    {
        case 0:
            phase_1 = State::HIGH;
            edge_1_callback(Edge::RISING, next_activation_time);
            next_activation_time += 700;
            break;
        case 1:
            phase_1 = State::LOW;
            edge_1_callback(Edge::FALLING, next_activation_time);
            next_activation_time += 200;
            break;
        case 2:
            phase_2 = State::HIGH;
            edge_2_callback(Edge::RISING, next_activation_time);
            next_activation_time += 550;
            break;
        case 3:
            phase_2 = State::LOW;
            edge_2_callback(Edge::FALLING, next_activation_time);
            next_activation_time += 550;
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
