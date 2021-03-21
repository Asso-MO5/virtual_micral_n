#include "IOController.h"

#include <devices/src/CPU8008.h>
#include <devices/src/Pluribus.h>

#include <utility>

IOController::IOController(const CPU8008& cpu, std::shared_ptr<Pluribus> pluribus)
    : cpu{cpu}, pluribus{std::move(pluribus)}
{}

void IOController::signal_phase_1(const Edge& edge)
{
    if (is_falling(edge))
    {
        auto cycle_control = static_cast<Constants8008::CycleControl>(latched_cycle_control);
        if (cycle_control == Constants8008::CycleControl::PCC &&
            *cpu.get_output_pins().sync == State::HIGH &&
            *cpu.get_output_pins().state == Constants8008::CpuState::T3)
        {
            if (will_emit)
            {
                auto time = edge.time();
                pluribus->data_bus_md0_7.request(this, time);
                pluribus->data_bus_md0_7.set(data_to_send, time, this);
            }
        }
    }
}

void IOController::signal_phase_2(const Edge& edge)
{
    if (is_falling(edge))
    {
        auto cycle_control = static_cast<Constants8008::CycleControl>(latched_cycle_control);

        if (cycle_control == Constants8008::CycleControl::PCC &&
            *cpu.get_output_pins().sync == State::HIGH &&
            *cpu.get_output_pins().state == Constants8008::CpuState::T3)
        {
            if (will_emit)
            {
                pluribus->data_bus_md0_7.release(this, edge.time());
                will_emit = false;
            }
        }
    }
}

void IOController::signal_sync(const Edge& edge)
{
    if (is_falling(edge))
    {
        read_io_information_from_cpu();
    }
}

void IOController::read_io_information_from_cpu()
{
    if (*cpu.get_output_pins().state == Constants8008::CpuState::T1 ||
        *cpu.get_output_pins().state ==
                Constants8008::CpuState::T1I) // While waiting for instruction Jam
    {
        latched_io_reg_A = cpu.data_pins.get_value();
    }
    if (*cpu.get_output_pins().state == Constants8008::CpuState::T2)
    {
        auto read_value = cpu.data_pins.get_value();

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
