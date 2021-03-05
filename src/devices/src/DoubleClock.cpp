#include "DoubleClock.h"

namespace Timings
{
    const Scheduling::counter_type PULSE_WIDTH_PHASE_1 = 700;
    const Scheduling::counter_type PULSE_WIDTH_PHASE_2 = 550;
    const Scheduling::counter_type DELAY_FROM_PHASE_1_TO_2 = 200;
} // namespace Timings

DoubleClock::DoubleClock(Frequency frequency) : main_frequency(frequency) {
    phase_1.request(this);
    phase_2.request(this);
}

void DoubleClock::step()
{
    auto next_activation_time = get_next_activation_time();
    switch (phase_count)
    {
        case 0:
            phase_1.set(State::HIGH, next_activation_time, this);
            next_phase_1 = next_activation_time + main_frequency.get_period_as_ns();
            next_activation_time += Timings::PULSE_WIDTH_PHASE_1;
            break;
        case 1:
            phase_1.set(State::LOW, next_activation_time, this);
            next_activation_time += Timings::DELAY_FROM_PHASE_1_TO_2;
            break;
        case 2:
            phase_2.set(State::HIGH, next_activation_time, this);
            next_activation_time += Timings::PULSE_WIDTH_PHASE_2;
            break;
        case 3:
            phase_2.set(State::LOW, next_activation_time, this);
            next_activation_time = next_phase_1;
            break;
    }
    set_next_activation_time(next_activation_time);
    phase_count = (phase_count + 1) % 4;
}

State DoubleClock::get_phase_1_state() const { return *phase_1; }

State DoubleClock::get_phase_2_state() const { return *phase_2; }

void DoubleClock::register_phase_1_trigger(const std::function<void(Edge)>& callback)
{
    phase_1.subscribe(callback);
}
void DoubleClock::register_phase_2_trigger(const std::function<void(Edge)>& callback)
{
    phase_2.subscribe(callback);
}
