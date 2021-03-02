#include "InterruptAtStart.h"

#include <devices/src/CPU8008.h>

InterruptAtStart::InterruptAtStart(std::shared_ptr<CPU8008> cpu) : cpu(std::move(cpu)) {}

void InterruptAtStart::signal_phase_1(const Edge& edge)
{
    if (is_rising(edge))
    {
        counter += 1;
        if (counter == 20)
        {
            cpu->signal_interrupt({Edge::Front::RISING, edge.time()});
        }
        if (counter == 21)
        {
            cpu->signal_interrupt(Edge{Edge::Front::FALLING, edge.time()});
        }
    }
}

void InterruptAtStart::signal_vdd(const Edge& edge)
{
    if (is_rising(edge))
    {
        counter = 0;
    }
}
