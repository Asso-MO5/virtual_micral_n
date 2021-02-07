#include "ControlBus.h"

#include <devices/src/CPU8008.h>
#include <devices/src/SimpleROM.h>

ControlBus::ControlBus(std::shared_ptr<CPU8008> cpu, std::shared_ptr<SimpleROM> rom)
    : cpu(std::move(cpu)), rom(std::move(rom))
{}

void ControlBus::signal_phase_1(const Edge& edge)
{
    if (edge == Edge::Front::RISING)
    {
        if (cpu->get_output_pins().sync == State::HIGH &&
            cpu->get_output_pins().state == CPU8008::CpuState::T3)
        {
            rom_output_disable(edge);
        }
    }
    else
    {
        if (cpu->get_output_pins().sync == State::HIGH &&
            cpu->get_output_pins().state == CPU8008::CpuState::T3)
        {
            rom_output_enable(edge);
        }
    }
}

void ControlBus::signal_phase_2(const Edge& edge)
{
    if (edge == Edge::Front::RISING)
    {
        read_address_from_cpu();
    }
    else
    {
    }
}

void ControlBus::read_address_from_cpu()
{
    if (cpu->get_output_pins().sync == State::LOW &&
        cpu->get_output_pins().state == CPU8008::CpuState::T1)
    {
        rom_address_bus |= cpu->get_data_pins().data;
    }
    if (cpu->get_output_pins().sync == State::LOW &&
        cpu->get_output_pins().state == CPU8008::CpuState::T2)
    {
        rom_address_bus |= (cpu->get_data_pins().data & 0x3f) << 8;
        rom->set_address(rom_address_bus);
    }
}

void ControlBus::rom_output_enable(const Edge& edge)
{
    assert(cpu->get_data_pins().taken == false);
    rom->signal_chip_select(Edge{Edge::Front::RISING, edge.time()});
    rom->signal_output_enable(Edge{Edge::Front::RISING, edge.time()});
}

void ControlBus::rom_output_disable(const Edge& edge)
{
    assert(cpu->get_data_pins().taken == false);
    rom->signal_output_enable(Edge{Edge::Front::FALLING, edge.time()});
    rom->signal_chip_select(Edge{Edge::Front::FALLING, edge.time()});
}
