#include "InterruptController.h"

#include "CPU8008.h"
#include "Pluribus.h"

InterruptController::InterruptController(std::shared_ptr<Pluribus> pluribus,
                                         std::shared_ptr<CPU8008> cpu)
    : pluribus{std::move(pluribus)}, cpu{std::move(cpu)},
      pluribus_int_ack{&this->pluribus->rzgi,  &this->pluribus->aint1, &this->pluribus->aint2,
                       &this->pluribus->aint3, &this->pluribus->aint4, &this->pluribus->aint5,
                       &this->pluribus->aint6, &this->pluribus->aint7}
{
    request_signals();
    connect_values();
}

void InterruptController::request_signals()
{
    for (auto& ack_line : pluribus_int_ack)
    {
        ack_line->request(this);
    }
}

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
    if (is_rising(edge) && !is_instruction_protected())
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
void InterruptController::reset_lowest_interrupt() { reset_interrupt(lowest_level_interrupt()); }

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

        if (has_a_requested_interrupt())
        {
            auto interrupt_level_to_rise = lowest_level_interrupt();
            assert(interrupt_level_to_rise < pluribus_int_ack.size());
            pluribus_int_ack[interrupt_level_to_rise]->set(State::HIGH, time, this);
        }

        start_instruction_protection();
    }
    else if (old_state == Constants8008::CpuState::T1I)
    {
        auto interrupt_level_to_lower = lowest_level_interrupt();
        // The inequality is based on the implementation of lowest_level_interrupt()... not very good
        if (interrupt_level_to_lower < pluribus_int_ack.size())
        {
            pluribus_int_ack[interrupt_level_to_lower]->set(State::LOW, time, this);
        }

        applying_interrupt = false;
    }

    if (new_state == Constants8008::CpuState::T1)
    {
        update_instruction_protection();
    }
}

void InterruptController::start_instruction_protection() { instruction_protection = 1; }
bool InterruptController::is_instruction_protected() const { return instruction_protection > 0; }
void InterruptController::update_instruction_protection()
{
    if (instruction_protection > 0)
    {
        instruction_protection = instruction_protection - 1;
    }
}
