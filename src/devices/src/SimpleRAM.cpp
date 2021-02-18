#include "SimpleRAM.h"

#include <utility>

// This simple RAM only fakes the Write Enable. In fact, it can read at any time as long
// as it's not "output enable". Even when not selected.

SimpleRAM::SimpleRAM(size_t size)
{
    data.resize(size);
    set_next_activation_time(Scheduling::unscheduled()); // The device is passive and immediate.
}

void SimpleRAM::connect_data_bus(std::shared_ptr<DataBus> bus)
{
    data_pins.connect(std::move(bus));
}

void SimpleRAM::step() {}

void SimpleRAM::signal_chip_select(Edge edge)
{
    input_pins.chip_select = edge.apply();
    set_data_when_read_selected();
}

void SimpleRAM::signal_output_enable(Edge edge)
{
    input_pins.output_enable = edge.apply();
    set_data_when_read_selected();
}

void SimpleRAM::signal_write_enable(Edge edge)
{
    input_pins.write_enable = edge.apply();

    if ((input_pins.write_enable == State::LOW) && (input_pins.chip_select == State::HIGH))
    {
        data[input_pins.address] = data_pins.read();
    }
    set_data_when_read_selected();
}

void SimpleRAM::set_data_when_read_selected()
{
    if ((input_pins.chip_select == State::HIGH) && (input_pins.output_enable == State::HIGH) &&
        (input_pins.write_enable == State::LOW))
    {
        // Simple RAM is immediate
        data_pins.take_bus();
        data_pins.write(data[input_pins.address]);
    }
    else
    {
        if (data_pins.is_owning_bus())
        {
            data_pins.release_bus();
        }
    }
}

uint8_t SimpleRAM::get_direct_data(uint16_t address) { return data[address]; }
void SimpleRAM::set_direct_data(uint16_t address, uint8_t new_data)
{
    if (address < data.size())
    {
        data[address] = new_data;
    }
}

ConnectedData& SimpleRAM::get_data_pins() { return data_pins; }

void SimpleRAM::set_address(uint16_t address) { input_pins.address = address % data.size(); }
