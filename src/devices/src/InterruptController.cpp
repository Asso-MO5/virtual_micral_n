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

void InterruptController::on_phase_1(const Edge& edge)
{
    if (is_rising(edge))
    {
        if (is_high(pluribus->init))
        {
            pending_int_level_0 = true;
        }

        if (!applying_interrupt)
        {
            if (pending_int_level_0)
            {
                applying_interrupt = true;
                cpu->input_pins.interrupt.request(this);
                cpu->input_pins.interrupt.set(State::HIGH, edge.time(), this);
            }
        }
    }
}

bool InterruptController::has_instruction_to_inject() const { return pending_int_level_0; }
void InterruptController::reset_interrupt(uint8_t) { pending_int_level_0 = false; }

uint8_t InterruptController::get_instruction_to_inject() const
{
    return 0x05; // RST $00
}

void InterruptController::cpu_state_changed(Constants8008::CpuState old_state,
                                            Constants8008::CpuState new_state,
                                            Scheduling::counter_type time)
{
    if (new_state == Constants8008::CpuState::T1I)
    {
        if (applying_interrupt)
        {
            cpu->input_pins.interrupt.set(State::LOW, time, this);
            cpu->input_pins.interrupt.release(this);
        }
        pluribus->rzgi.set(State::HIGH, time, this);
    }
    else if (old_state == Constants8008::CpuState::T1I)
    {
        pluribus->rzgi.set(State::LOW, time, this);
        applying_interrupt = false;
    }
}
