#include "ConsoleCard.h"
#include "PluribusHelpers.h"

#include <devices/src/Pluribus.h>
#include <emulation_core/src/DataBus.h>

#include <utility>

ConsoleCard::ConsoleCard(std::shared_ptr<Pluribus> given_pluribus,
                         ConsoleCard::StartMode start_mode)
    : pluribus{std::move(given_pluribus)}, start_mode{start_mode}
{
    set_next_activation_time(Scheduling::unscheduled());
    pluribus->ready_console.request(this);

    pluribus->phase_2.subscribe([this](Edge edge) { on_phase_2(edge); });
    pluribus->t3.subscribe([this](Edge edge) { on_t3(edge); });
    pluribus->sync.subscribe([this](Edge edge) { on_sync(edge); });
    pluribus->vdd.subscribe([this](Edge edge) { on_vdd(edge); });
}

ConsoleCard::Status ConsoleCard::get_status() const { return status; }

void ConsoleCard::step() {}

void ConsoleCard::press_automatic()
{
    status.automatic = true;
    status.stepping = false;

    auto time = pluribus->phase_2.get_latest_change_time();
    pluribus->ready_console.set(State::HIGH, time, this);
}

void ConsoleCard::set_step_mode()
{
    status.automatic = false;
    status.stepping = true;
}

void ConsoleCard::press_stepping()
{
    set_step_mode();

    auto time = pluribus->phase_2.get_latest_change_time();
    pluribus->ready_console.set(State::HIGH, time, this);
}

void ConsoleCard::press_trap() { status.trap = !status.trap; }

void ConsoleCard::on_vdd(Edge edge)
{
    if (is_rising(edge))
    {
        if (start_mode == Automatic)
        {
            press_automatic();
        }
        else
        {
            press_stepping();
        }
    }
}

void ConsoleCard::on_t3(Edge edge) {}

void ConsoleCard::press_instruction() { status.step_mode = Instruction; }
void ConsoleCard::press_cycle() { status.step_mode = Cycle; }
void ConsoleCard::on_sync(Edge edge)
{
    status.is_waiting = *pluribus->wait == State::HIGH;
    status.is_stopped = *pluribus->stop == State::HIGH;

    if (is_falling(edge) && is_high(*pluribus->t2))
    {
        if (status.trap && !status.stepping)
        {
            pluribus->ready_console.set(State::LOW, edge.time(), this);
        }
    }
}

void ConsoleCard::on_phase_2(Edge edge)
{
    if (is_falling(edge))
    {
        auto time = edge.time();
        if (is_high(*pluribus->t2))
        {
            if (status.stepping)
            {
                auto cc0 = *pluribus->cc0;
                auto cc1 = *pluribus->cc1;
                Constants8008::CycleControl cycleControl = cycle_control_from_cc(cc0, cc1);

                status.is_op_cycle = cycleControl == Constants8008::CycleControl::PCI;
                status.is_read_cycle = cycleControl == Constants8008::CycleControl::PCR;
                status.is_io_cycle = cycleControl == Constants8008::CycleControl::PCC;
                status.is_write_cycle = cycleControl == Constants8008::CycleControl::PCW;

                switch (status.step_mode)
                {
                    case Instruction:
                        if (status.is_op_cycle)
                        {
                            pluribus->ready_console.set(State::LOW, time, this);
                        }
                        else
                        {
                            pluribus->ready_console.set(State::HIGH, time, this);
                        }
                        break;
                    case Cycle:
                        pluribus->ready_console.set(State::LOW, time, this);
                        break;
                }
            }
        }

        if (is_high(*pluribus->t3prime))
        {
            status.data = pluribus->data_bus_md0_7.get_value();
            status.address = *pluribus->address_bus_s0_s13;

            if (status.trap && status.is_op_cycle)
            {
                if (status.address == switch_address)
                {
                    set_step_mode();
                }
                else
                {
                    if (!status.stepping)
                    {
                        pluribus->ready_console.set(State::HIGH, time, this);
                    }
                }
            }
        }
    }
}

void ConsoleCard::set_switch_data(uint8_t data) { switch_data = data; }
void ConsoleCard::set_switch_address(uint16_t address) { switch_address = address; }
