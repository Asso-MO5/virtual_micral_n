#include "AutomaticStart.h"

#include <devices/src/CPU8008.h>

AutomaticStart::AutomaticStart(std::shared_ptr<CPU8008> cpu) : cpu(std::move(cpu)) {}

void AutomaticStart::signal_phase_1(const Edge& edge)
{
    if (is_rising(edge))
    {
        counter += 1;
        if (counter == 20)
        {
            cpu->signal_interrupt({Edge::Front::RISING, edge.time()});
        }
        if (counter == 22)
        {
            cpu->signal_interrupt(Edge{Edge::Front::FALLING, edge.time()});
        }
    }
}

void AutomaticStart::signal_vdd(const Edge& edge)
{
    counter = 0;
}
