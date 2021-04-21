#include "AutomaticStart.h"

#include <devices/src/CPU8008.h>

AutomaticStart::AutomaticStart(std::shared_ptr<CPU8008> cpu) : cpu(std::move(cpu)) {
}

void AutomaticStart::signal_phase_1(const Edge& edge)
{
    if (is_rising(edge))
    {
        counter += 1;
        if (counter == 20)
        {
            cpu->input_pins.interrupt.request(this);
            cpu->input_pins.interrupt.set(State::HIGH, edge.time(), this);
        }
        if (counter == 22)
        {
            cpu->input_pins.interrupt.set(State::LOW, edge.time(), this);
            cpu->input_pins.interrupt.release(this);
        }
    }
}

void AutomaticStart::signal_vdd(const Edge& edge)
{
    counter = 0;
}
