#include "InterruptController.h"

#include "CPU8008.h"

InterruptController::InterruptController(std::shared_ptr<CPU8008> cpu) : cpu{std::move(cpu)} {}

void InterruptController::signal_phase_1(const Edge& edge)
{
    if (is_rising(edge))
    {
        if (interrupt_is_scheduled && !applying_interrupt)
        {
            interrupt_is_scheduled = false;
            applying_interrupt = true;
            interrupt_callback(edge);
        }
    }
    else
    {
        if (applying_interrupt && (*cpu->output_pins.state == Constants8008::CpuState::T1I))
        {
            interrupt_is_scheduled = false;
            applying_interrupt = false;
            interrupt_callback(edge);
        }
    }
}

void InterruptController::register_interrupt_trigger(std::function<void(Edge)> callback)
{
    interrupt_callback = std::move(callback);
}

void InterruptController::on_init_changed(const Edge& edge)
{
    if (is_rising(edge))
    {
        interrupt_is_scheduled = true;
    }
}
