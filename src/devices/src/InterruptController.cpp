#include "InterruptController.h"

#include "CPU8008.h"
#include "Pluribus.h"

InterruptController::InterruptController(std::shared_ptr<Pluribus> pluribus,
                                         std::shared_ptr<CPU8008> cpu)
    : pluribus{std::move(pluribus)}, cpu{std::move(cpu)}
{
    request_signals();
    connect_values();
}

void InterruptController::request_signals() { pluribus->rzgi.request(this); }

void InterruptController::connect_values()
{
    cpu->register_state_change([this](Constants8008::CpuState old_state,
                                      Constants8008::CpuState new_state,
                                      Scheduling::counter_type time) {
        cpu_state_changed(old_state, new_state, time);
    });
}

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
}

bool InterruptController::has_instruction_to_inject() const { return false; }
void InterruptController::cpu_state_changed(Constants8008::CpuState old_state,
                                            Constants8008::CpuState new_state,
                                            Scheduling::counter_type time)
{
    if (new_state == Constants8008::CpuState::T1I)
    {
        cpu->signal_interrupt(Edge{Edge::Front::FALLING, time});
        pluribus->rzgi.set(State::HIGH, time, this);
    }
    else if (old_state == Constants8008::CpuState::T1I)
    {
        pluribus->rzgi.set(State::LOW, time, this);
        applying_interrupt = false;
    }
}
