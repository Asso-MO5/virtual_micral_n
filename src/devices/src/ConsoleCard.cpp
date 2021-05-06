#include "ConsoleCard.h"

#include "Pluribus.h"

#include <utility>

namespace
{
    // A bit higher than the number of sample when at 60 FPS
    // When running at lower rate, consider an immediate display mode on the PanelControl
    // or raising this value.
    // This could be also made configurable.
    const size_t HISTORY_SIZE = 5000;
} // namespace

ConsoleCard::ConsoleCard(std::shared_ptr<Pluribus> given_pluribus,
                         ConsoleCard::StartMode start_mode, RecordMode record_mode)
    : start_mode{start_mode}, pluribus{std::move(given_pluribus)},
      status_history{record_mode == DontRecord ? 1 : HISTORY_SIZE}
{
    set_next_activation_time(Scheduling::unscheduled());
    pluribus->ready_console.request(this);

    pluribus->phase_2.subscribe([this](Edge edge) { on_phase_2(edge); });
    pluribus->sync.subscribe([this](Edge edge) { on_sync(edge); });
    pluribus->vdd.subscribe([this](Edge edge) { on_vdd(edge); });
    pluribus->rzgi.subscribe([this](Edge edge) { on_rzgi(edge); });
}

ConsoleCard::Status ConsoleCard::get_status() const { return status; }

const ConsoleCard::StatusHistory::StatusContainer& ConsoleCard::get_status_history() const
{
    return status_history.get_history();
}

void ConsoleCard::reset_history()
{
    status_history.reset();
    status_history.push(status);
}

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

void ConsoleCard::press_instruction() { status.step_mode = Instruction; }
void ConsoleCard::press_cycle() { status.step_mode = Cycle; }
void ConsoleCard::on_sync(Edge edge)
{
    status.is_waiting = *pluribus->wait == State::HIGH;
    status.is_stopped = *pluribus->stop == State::HIGH;

    if (is_rising(edge))
    {
        status_history.push(status);
    }
}

void ConsoleCard::on_phase_2(Edge edge)
{
    if (is_falling(edge))
    {
        auto time = edge.time();
        if (is_high(pluribus->t2) && is_low(pluribus->sync))
        {
            auto cc0 = *pluribus->cc0;
            auto cc1 = *pluribus->cc1;
            Constants8008::CycleControl cycleControl = cycle_control_from_cc(cc0, cc1);

            status.is_op_cycle = cycleControl == Constants8008::CycleControl::PCI;
            status.is_read_cycle = cycleControl == Constants8008::CycleControl::PCR;
            status.is_io_cycle = cycleControl == Constants8008::CycleControl::PCC;
            status.is_write_cycle = cycleControl == Constants8008::CycleControl::PCW;
            status.address = *pluribus->address_bus_s0_s13;

            if (status.stepping)
            {
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
            else if (status.trap)
            {
                if (status.is_op_cycle && (status.address == switch_address))
                {
                    pluribus->ready_console.set(State::LOW, edge.time(), this);
                    set_step_mode();
                }
            }
        }

        if (is_high(pluribus->t3prime))
        {
            status.data = pluribus->data_bus_md0_7.get_value();
        }
    }
    else
    {
        if (pending_interrupt)
        {
            auto time = edge.time();

            pluribus->init.request(this);
            pluribus->init.set(State::HIGH, time, this);

            pending_interrupt = false;
        }
    }
}

void ConsoleCard::set_switch_data(uint8_t data) { switch_data = data; }
void ConsoleCard::set_switch_address(uint16_t address) { switch_address = address; }
void ConsoleCard::press_interrupt() { pending_interrupt = true; }

void ConsoleCard::on_rzgi(Edge edge)
{
    if (is_falling(edge) && is_high(pluribus->init))
    {
        pluribus->init.set(State::LOW, edge.time(), this);
        pluribus->init.release(this);
    }
}

ConsoleCard::StatusHistory::StatusHistory(size_t size) { history.reserve(size); }

void ConsoleCard::StatusHistory::reset() { history.clear(); }
void ConsoleCard::StatusHistory::push(const ConsoleCard::Status& status_to_record)
{
    if (history.size() < history.capacity())
    {
        history.push_back(status_to_record);
    }
}
const ConsoleCard::StatusHistory::StatusContainer& ConsoleCard::StatusHistory::get_history() const
{
    return history;
}
