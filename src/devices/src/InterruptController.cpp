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

void InterruptController::read_required_int_from_bus(OwnedSignal& signal, uint8_t level)
{
    requested_interrupts[level] = requested_interrupts[level] | is_high(signal);
}

void InterruptController::on_phase_1(const Edge& edge)
{
    if (is_rising(edge))
    {
        read_required_int_from_bus(pluribus->init, 0);
        read_required_int_from_bus(pluribus->bi1, 1);
        read_required_int_from_bus(pluribus->bi2, 2);
        read_required_int_from_bus(pluribus->bi3, 3);
        read_required_int_from_bus(pluribus->bi4, 4);
        read_required_int_from_bus(pluribus->bi5, 5);
        read_required_int_from_bus(pluribus->bi6, 6);
        read_required_int_from_bus(pluribus->bi7, 7);

        if (!applying_interrupt)
        {
            if (has_a_requested_interrupt())
            {
                applying_interrupt = true;
                cpu->input_pins.interrupt.request(this);
                cpu->input_pins.interrupt.set(State::HIGH, edge.time(), this);
            }
        }
    }
}

bool InterruptController::has_a_requested_interrupt() const
{
    return std::any_of(begin(requested_interrupts), end(requested_interrupts),
                       [](auto b) { return b; });
}

uint8_t InterruptController::lowest_level_interrupt() const
{
    return static_cast<uint8_t>(std::find_if(begin(requested_interrupts), end(requested_interrupts),
                                             [](auto b) { return b; }) -
                                begin(requested_interrupts));
}

bool InterruptController::has_instruction_to_inject() const { return has_a_requested_interrupt(); }
void InterruptController::reset_interrupt(uint8_t level) { requested_interrupts[level] = false; }
void InterruptController::reset_lowest_interrupt() {reset_interrupt(lowest_level_interrupt());}

uint8_t InterruptController::get_instruction_to_inject() const
{
    assert(has_instruction_to_inject() &&
           "No instruction to inject. Asking for the instruction is invalid.");
    return 0x05 | ((lowest_level_interrupt() << 3) & 0b00111000); // RST
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
