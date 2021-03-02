#include "ConsoleCard.h"

#include <devices/src/Pluribus.h>
#include <emulation_core/src/DataBus.h>

#include <utility>

ConsoleCard::ConsoleCard(std::shared_ptr<Pluribus> given_pluribus)
    : pluribus{std::move(given_pluribus)}
{
    set_next_activation_time(Scheduling::unscheduled());
    pluribus->ready_console.request(this);

    pluribus->t3.subscribe([this](Edge edge) { on_t3(edge); });
    pluribus->sync.subscribe([this](Edge edge) { on_sync(edge); });

    press_automatic();
}

ConsoleCard::Status ConsoleCard::get_status() const { return status; }

void ConsoleCard::step() { status.address = *pluribus->address_bus_s0_s13; }

void ConsoleCard::press_automatic()
{
    status.automatic = true;
    status.stepping = false;

    auto time = pluribus->phase_2.get_latest_change_time();
    pluribus->ready_console.set(State::HIGH, time, this);
}

void ConsoleCard::press_stepping()
{
    status.automatic = false;
    status.stepping = true;

    auto time = pluribus->phase_2.get_latest_change_time();
    pluribus->ready_console.set(State::HIGH, time, this);
}

void ConsoleCard::press_trap() {}

void ConsoleCard::on_t3(Edge edge)
{
    if (status.stepping && is_rising(edge))
    {
        auto time = edge.time();
        switch (status.step_mode)
        {
            case Instruction:
                if (is_low(*pluribus->cc0) && is_low(*pluribus->cc1))
                {
                    pluribus->ready_console.set(State::LOW, time, this);
                }
                break;
            case Cycle:
                pluribus->ready_console.set(State::LOW, time, this);
                break;
        }

        status.is_op_cycle = is_low(*pluribus->cc0) && is_low(*pluribus->cc1);
        status.is_read_cycle = is_low(*pluribus->cc0) && is_high(*pluribus->cc1);
        status.is_io_cycle = is_high(*pluribus->cc0) && is_low(*pluribus->cc1);
        status.is_write_cycle = is_high(*pluribus->cc0) && is_high(*pluribus->cc1);
    }
}

void ConsoleCard::press_instruction() { status.step_mode = Instruction; }
void ConsoleCard::press_cycle() { status.step_mode = Cycle; }
void ConsoleCard::on_sync(Edge edge)
{
    if (is_falling(edge))
    {
        if (is_high(*pluribus->t3) && is_low(*pluribus->cc0) && is_high(*pluribus->cc1))
        {
            // TODO: Check when the read timing is done.
            status.data = pluribus->data_bus_d0_7->read();
        }
    }
}
