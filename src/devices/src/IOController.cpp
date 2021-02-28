#include "IOController.h"

#include <devices/src/CPU8008.h>
#include <emulation_core/src/DataBus.h>

#include <utility>

IOController::IOController(std::shared_ptr<CPU8008> cpu, std::shared_ptr<DataBus> bus)
    : cpu{std::move(cpu)}, bus{std::move(bus)}
{
    latched_io_data.connect(this->bus);
}

void IOController::signal_phase_1(const Edge& edge)
{
    if (edge == Edge::Front::FALLING)
    {
        auto cycle_control = static_cast<Constants8008::CycleControl>(latched_cycle_control);
        if (cycle_control == Constants8008::CycleControl::PCC &&
            cpu->get_output_pins().sync == State::HIGH &&
            *cpu->get_output_pins().state == Constants8008::CpuState::T3)
        {
            if (will_emit)
            {
                latched_io_data.take_bus();
                latched_io_data.write(data_to_send); // Fake Data at the moment
            }
        }
    }
}

void IOController::signal_phase_2(const Edge& edge)
{
    if (edge == Edge::Front::FALLING)
    {
        auto cycle_control = static_cast<Constants8008::CycleControl>(latched_cycle_control);

        if (cycle_control == Constants8008::CycleControl::PCC &&
            cpu->get_output_pins().sync == State::HIGH &&
            *cpu->get_output_pins().state == Constants8008::CpuState::T3)
        {
            if (will_emit)
            {
                latched_io_data.release_bus();
                will_emit = false;
            }
        }
    }
}

void IOController::signal_sync(const Edge& edge)
{
    if (edge == Edge::Front::FALLING)
    {
        read_io_information_from_cpu();
    }
}

void IOController::read_io_information_from_cpu()
{
    if (*cpu->get_output_pins().state == Constants8008::CpuState::T1 ||
        *cpu->get_output_pins().state ==
                Constants8008::CpuState::T1I) // While waiting for instruction Jam
    {
        latched_io_reg_A = cpu->get_data_pins().read();
    }
    if (*cpu->get_output_pins().state == Constants8008::CpuState::T2)
    {
        auto read_value = cpu->get_data_pins().read();

        latched_io_reg_b = read_value;
        latched_cycle_control = read_value & 0b11000000;

        auto cycle_control = static_cast<Constants8008::CycleControl>(latched_cycle_control);
        if (cycle_control == Constants8008::CycleControl::PCC)
        {
            process_io();
        }
    }
}

void IOController::process_io()
{
    assert((latched_io_reg_b & 1) && "Must have received INP our OUT instruction.");
    bool is_OUT = (latched_io_reg_b & 0b00110000);

    if (is_OUT)
    {
        received_data = latched_io_reg_A;
        // TODO: Should schedule a WAIT signal at next T3
    }
    else
    {
        will_emit = true;
    }
}

void IOController::set_data_to_send(uint8_t data) { data_to_send = data; }
uint8_t IOController::get_received_data() const { return received_data; }
