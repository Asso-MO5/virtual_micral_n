#include "CPU8008.h"
#include <emulation_core/src/Edge.h>

namespace
{
    const Scheduling::counter_type BOOT_UP_TIME = 16'000;
}

CPU8008::CPU8008() { output_pins.state = State::STOPPED; }

void CPU8008::step() {}

Scheduling::counter_type CPU8008::get_next_activation_time() const { return next_activation_time; }

const CPU8008::OutputPins& CPU8008::get_output_pins() const { return output_pins; }
const CPU8008::DataPins& CPU8008::get_data_pins() const { return data_pins; }

void CPU8008::signal_phase_1(Edge edge, Scheduling::counter_type time)
{
    if (input_pins.vdd == ::State::LOW)
    {
        next_activation_time = Scheduling::unscheduled();
        return;
    }

    next_activation_time = time;

    if (edge == Edge::RISING)
    {
        if ((time - clock_1_count) > BOOT_UP_TIME && input_pins.vdd == ::State::HIGH &&
            input_pins.interrupt == ::State::HIGH)
        {
            output_pins.state = State::T1I;
        }
    }
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
