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
            auto cycle_control = static_cast<Constants8008::CycleControl>(latched_cycle_control);

            if (cycle_control == Constants8008::CycleControl::PCI ||
                cycle_control == Constants8008::CycleControl::PCR)
            {
                rom_output_disable(edge);
            }
        }
    }
    else
    {
        if (cpu->get_output_pins().sync == State::HIGH &&
            cpu->get_output_pins().state == CPU8008::CpuState::T3)
        {
            auto cycle_control = static_cast<Constants8008::CycleControl>(latched_cycle_control);

            if (cycle_control == Constants8008::CycleControl::PCI ||
                cycle_control == Constants8008::CycleControl::PCR)
            {

                rom_output_enable(edge);
            }
        }
    }
}

void ControlBus::signal_phase_2(const Edge& edge)
{
    if (edge == Edge::Front::FALLING && cpu->get_output_pins().sync == State::HIGH) {}
}

void ControlBus::signal_sync(const Edge& edge)
{
    if (edge == Edge::Front::RISING)
    {
        if (cpu->get_output_pins().state == CPU8008::CpuState::T3) {}
    }
    else
    {
        if (cpu->get_output_pins().state == CPU8008::CpuState::T3)
        {
            // TODO: Only on PCI and PCR
            // Or maybe set DATA OUT only on Sync Low
            // Or Latch Cycle Control on T2 (PCI + Sync + T2)
            // Record the ROM Enable signal
            // Display the T1... and PCI...
            // Make a Panel for the Signals
            //rom_output_disable(edge);
        }
        read_address_from_cpu();
    }
}

void ControlBus::read_address_from_cpu()
{
    if (cpu->get_output_pins().state == CPU8008::CpuState::T1)
    {
        // TODO: Should be replaced by a decoder
        rom_address_bus &= 0xff00;
        rom_address_bus |= cpu->get_data_pins().read();
    }
    if (cpu->get_output_pins().state == CPU8008::CpuState::T2)
    {
        auto read_value = cpu->get_data_pins().read();
        // TODO: Should be replaced by a decoder
        rom_address_bus &= 0x00ff;
        rom_address_bus |= (read_value & 0x3f) << 8;

        // TODO: Only on PCI, PCR and PCW ??
        rom->set_address(rom_address_bus);

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
