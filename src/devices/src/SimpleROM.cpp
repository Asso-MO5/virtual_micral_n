
#include "SimpleROM.h"

#include <utility>

SimpleROM::SimpleROM(std::vector<uint8_t> data) : data{std::move(data)}
{
    set_next_activation_time(Scheduling::unscheduled()); // The device is passive and immediate.
}

void SimpleROM::connect_data_bus(std::shared_ptr<DataBus> bus)
{
    data_pins.connect(std::move(bus));
}

void SimpleROM::step() {}

void SimpleROM::signal_chip_select(Edge edge)
{
    input_pins.chip_select = edge.apply();
    set_data_when_read_selected();
}

void SimpleROM::signal_output_enable(Edge edge)
{
    input_pins.output_enable = edge.apply();
    set_data_when_read_selected();
}

void SimpleROM::set_data_when_read_selected()
{
    if ((input_pins.chip_select == State::HIGH) && (input_pins.output_enable == State::HIGH))
    {
        // Simple ROM is immediate
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

uint8_t SimpleROM::get_direct_data(uint16_t address) { return data[address]; }

const ConnectedData& SimpleROM::get_data_pins() const { return data_pins; }

void SimpleROM::set_address(uint16_t address) { input_pins.address = address; }
