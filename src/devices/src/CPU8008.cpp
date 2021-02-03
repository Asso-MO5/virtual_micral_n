#include "CPU8008.h"
#include <emulation_core/src/Edge.h>

namespace
{
    const Scheduling::counter_type BOOT_UP_TIME = 16'000;

    namespace Timings
    {
        const Scheduling::counter_type MIN_CLOCK_PERIOD = 2000;
        const Scheduling::counter_type MAX_CLOCK_PERIOD = 3000;
        const Scheduling::counter_type PULSE_WIDTH_PHASE_1 = 700;
        const Scheduling::counter_type PULSE_WIDTH_PHASE_2 = 550;
        const Scheduling::counter_type FALLING_1_TO_FALLING_2_EDGE_MIN = 900;
        const Scheduling::counter_type FALLING_1_TO_FALLING_2_EDGE_MAX = 1100;
        const Scheduling::counter_type DELAY_FROM_PHASE_1_TO_2 = 200;
        const Scheduling::counter_type DELAY_FROM_PHASE_2_TO_1 = 400;
    } // namespace Timings

} // namespace

CPU8008::CPU8008() { output_pins.state = State::STOPPED; }

void CPU8008::step() {}

const CPU8008::OutputPins& CPU8008::get_output_pins() const { return output_pins; }
const CPU8008::DataPins& CPU8008::get_data_pins() const { return data_pins; }

void CPU8008::signal_phase_1(Edge edge, Scheduling::counter_type time)
{
    if (input_pins.vdd == ::State::LOW)
    {
        set_next_activation_time(Scheduling::unscheduled());
        return;
    }

    set_next_activation_time(time);

    if (edge == Edge::RISING)
    {
        if ((time - clock_1_count) > BOOT_UP_TIME && input_pins.vdd == ::State::HIGH &&
            input_pins.interrupt == ::State::HIGH)
        {
            output_pins.state = State::T1I;
        }
    }
}

void CPU8008::signal_phase_2(Edge edge, Scheduling::counter_type time) {

}

void CPU8008::signal_vdd(Edge edge, Scheduling::counter_type time)
{
    if (edge == Edge::RISING)
    {
        clock_1_count = 0;
        input_pins.vdd = ::State::HIGH;
    }
}
void CPU8008::signal_interrupt(Edge edge, Scheduling::counter_type time)
{
    if (edge == Edge::RISING)
    {
        input_pins.interrupt = ::State::HIGH;
    }
    else
    {
        input_pins.interrupt = ::State::LOW;
    }
}
