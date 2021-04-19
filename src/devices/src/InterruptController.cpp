#include "InterruptController.h"

#include "CPU8008.h"
#include "Pluribus.h"

InterruptController::InterruptController(std::shared_ptr<Pluribus> pluribus,
                                         std::shared_ptr<CPU8008> cpu)
    : pluribus{std::move(pluribus)}, cpu{std::move(cpu)}
{}

void InterruptController::signal_phase_1(const Edge& edge)
{
    if (is_rising(edge))
    {
        if (is_high(*pluribus->init) && !applying_interrupt)
        {
            applying_interrupt = true;
            cpu->signal_interrupt(edge);
        }
    }
    else
    {
        if (applying_interrupt && (*cpu->output_pins.state == Constants8008::CpuState::T1I))
        {
            applying_interrupt = false;
            cpu->signal_interrupt(edge);
        }
    }
}
