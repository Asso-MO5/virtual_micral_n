#include "ControlBus.h"
#include "SimpleRAM.h"
#include "SimpleROM.h"

#include <devices/src/CPU8008.h>

namespace
{
    enum AddressDestination
    {
        ROM,
        RAM,
        None,
    };

    AddressDestination get_destination_from_address(uint16_t address)
    {
        if (address < 0x1000)
        {
            return ROM;
        }
        if (address >= 0x1000 & address < 0x1000 + 2048)
        {
            return RAM;
        }
        return None;
    }
} // namespace

ControlBus::ControlBus(std::shared_ptr<CPU8008> cpu, std::shared_ptr<SimpleROM> rom,
                       std::shared_ptr<SimpleRAM> ram)
    : cpu{std::move(cpu)}, rom{std::move(rom)}, ram{std::move(ram)}
{}

void ControlBus::signal_phase_1(const Edge& edge)
{
    if (edge == Edge::Front::RISING)
    {
        if (cpu->get_output_pins().sync == State::HIGH &&
            *cpu->get_output_pins().state == Constants8008::CpuState::T3)
        {
            stop_t3_transfer(edge);
        }
    }
    else
    {
        if (cpu->get_output_pins().sync == State::HIGH &&
            *cpu->get_output_pins().state == Constants8008::CpuState::T3)
        {
            start_t3_transfer(edge);
        }
    }
}

void ControlBus::stop_t3_transfer(const Edge& edge)
{
    auto cycle_control = static_cast<Constants8008::CycleControl>(latched_cycle_control);

    if (cycle_control == Constants8008::CycleControl::PCI ||
        cycle_control == Constants8008::CycleControl::PCR)
    {
        switch (get_destination_from_address(latched_address))
        {
            case ROM:
                rom_output_disable(edge);
                break;
            case RAM:
                ram_output_disable(edge);
                break;
            case None:
                break;
        }
    }
    else if (cycle_control == Constants8008::CycleControl::PCW)
    {
        if (get_destination_from_address(latched_address) == RAM)
        {
            ram_write_disable(edge);
        }
    }
    else if (cycle_control == Constants8008::CycleControl::PCC)
    {
        // TODO: Signal to thw I/O Controller for stop emitting
    }
}

void ControlBus::start_t3_transfer(const Edge& edge)
{
    auto cycle_control = static_cast<Constants8008::CycleControl>(latched_cycle_control);

    if (cycle_control == Constants8008::CycleControl::PCI ||
        cycle_control == Constants8008::CycleControl::PCR)
    {
        switch (get_destination_from_address(latched_address))
        {
            case ROM:
                rom_output_enable(edge);
                break;
            case RAM:
                ram_output_enable(edge);
                break;
            case None:
                break;
        }
    }
    else if (cycle_control == Constants8008::CycleControl::PCW)
    {
        if (get_destination_from_address(latched_address) == RAM)
        {
            ram_write_enable(edge);
        }
    }
    else if (cycle_control == Constants8008::CycleControl::PCC)
    {
        // TODO: Signal to thw I/O Controller for start emitting
    }
}

void ControlBus::signal_phase_2(const Edge& edge)
{
    if (edge == Edge::Front::FALLING && cpu->get_output_pins().sync == State::HIGH) {}
}

void ControlBus::signal_sync(const Edge& edge)
{
    if (edge == Edge::Front::FALLING)
    {
        read_address_from_cpu();
    }
}

void ControlBus::read_address_from_cpu()
{
    if (*cpu->get_output_pins().state == Constants8008::CpuState::T1 ||
        *cpu->get_output_pins().state ==
                Constants8008::CpuState::T1I) // While waiting for instruction Jam
    {
        // TODO: Should be replaced by a decoder
        latched_address &= 0xff00;
        latched_address |= cpu->get_data_pins().read();
    }
    if (*cpu->get_output_pins().state == Constants8008::CpuState::T2)
    {
        auto read_value = cpu->get_data_pins().read();
        // TODO: Should be replaced by a decoder
        latched_address &= 0x00ff;
        latched_address |= (read_value & 0x3f) << 8;

        // The address is always latched and applied, even on PCC cycle, it will just not be used.
        rom->set_address(latched_address & 0x0fff);
        ram->set_address(latched_address & 0x0fff);

        latched_cycle_control = read_value & 0b11000000;
    }
}

void ControlBus::rom_output_enable(const Edge& edge)
{
    rom->signal_chip_select(Edge{Edge::Front::RISING, edge.time()});
    rom->signal_output_enable(Edge{Edge::Front::RISING, edge.time()});
}

void ControlBus::rom_output_disable(const Edge& edge)
{
    rom->signal_output_enable(Edge{Edge::Front::FALLING, edge.time()});
    rom->signal_chip_select(Edge{Edge::Front::FALLING, edge.time()});
}

void ControlBus::ram_output_enable(const Edge& edge)
{
    ram->signal_chip_select(Edge{Edge::Front::RISING, edge.time()});
    ram->signal_output_enable(Edge{Edge::Front::RISING, edge.time()});
}

void ControlBus::ram_output_disable(const Edge& edge)
{
    ram->signal_output_enable(Edge{Edge::Front::FALLING, edge.time()});
    ram->signal_chip_select(Edge{Edge::Front::FALLING, edge.time()});
}

void ControlBus::ram_write_enable(const Edge& edge)
{
    ram->signal_chip_select(Edge{Edge::Front::RISING, edge.time()});
    ram->signal_write_enable(Edge{Edge::Front::RISING, edge.time()});
}

void ControlBus::ram_write_disable(const Edge& edge)
{
    ram->signal_write_enable(Edge{Edge::Front::FALLING, edge.time()});
    ram->signal_chip_select(Edge{Edge::Front::FALLING, edge.time()});
}
