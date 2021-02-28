#include "InterruptController.h"

InterruptController::InterruptController() {}

void InterruptController::wants_interrupt(const Edge& edge)
{
    if (edge == Edge::Front::RISING)
    {
        interrupt_is_scheduled = true;
    }
}

void InterruptController::signal_phase_1(const Edge& edge)
{
    if (edge == Edge::Front::RISING)
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
        if (applying_interrupt && (latest_cpu_state == Constants8008::CpuState::T1I))
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

void InterruptController::on_state_value_change(Constants8008::CpuState old_value,
                                                Constants8008::CpuState new_value,
                                                Scheduling::counter_type time)
{
    latest_cpu_state = new_value;
}
