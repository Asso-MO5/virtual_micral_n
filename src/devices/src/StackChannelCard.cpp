#include "StackChannelCard.h"

#include "Pluribus.h"

#include <iostream>

namespace
{
    const Scheduling::counter_type STACK_MEMORY_READ_DELAY = 150;

    constexpr bool is_input(uint16_t address) { return (address & 0b11000000000000) == 0; }
} // namespace

StackChannelCard::StackChannelCard(const StackChannelCard::Config& config)
    : scheduler{config.scheduler}, pluribus{config.pluribus}, configuration(config.configuration)
{
    assert(configuration.memory_size > 0 && "The memory size must not be 0");

    set_next_activation_time(Scheduling::unscheduled());
    set_data_size();

    pluribus->t2.subscribe([this](Edge edge) { on_t2((edge)); });
    pluribus->t3.subscribe([this](Edge edge) { on_t3((edge)); });
}

void StackChannelCard::step()
{
    // Note: currently, the only step action is to send data on md0-md7.
    // If other actions happen, this will have to be changed.
    auto time = get_next_activation_time();

    pluribus->data_bus_md0_7.set(latched_data, time, this);
    is_emitting_data = true;

    pluribus->ready.request(this);
    pluribus->ready.set(State::HIGH, time, this);

    set_next_activation_time(Scheduling::unscheduled());
}

void StackChannelCard::set_data_size() { data.resize(configuration.memory_size); }

void StackChannelCard::on_t2(Edge edge)
{
    if (is_falling(edge))
    {
        auto [address, cycle_control] = decode_address_on_bus(*pluribus);
        if ((cycle_control == Constants8008::CycleControl::PCC) && is_addressed(address))
        {
            if (is_input(address))
            {
                // This is the end of T2, schedule the data emission
                pop_data();
                pluribus->data_bus_md0_7.request(this, edge.time());
                set_next_activation_time(edge.time() + STACK_MEMORY_READ_DELAY);
                scheduler.change_schedule(get_id());
            }
            else
            {
                push_data(address);
            }
        }
    }
}
void StackChannelCard::on_t3(Edge edge)
{
    //TODO: duplicated code with MemoryCard, to extract
    //The whole "emitting a data on md0-md7" is the same.
    if (is_falling(edge) && is_emitting_data)
    {
        pluribus->data_bus_md0_7.release(this, edge.time());
        pluribus->ready.release(this);
    }
}

void StackChannelCard::pop_data()
{
    if (data_pointer > 0)
    {
        data_pointer -= 1;
    }
    else
    {
        data_pointer = data.size() - 1;
    }
    latched_data = data[data_pointer];
}

bool StackChannelCard::is_addressed(uint16_t address) const
{
    const uint8_t input_group = (address & 0b00111000000000) >> 9;
    const uint8_t output_address = ((address & 0b11111000000000) >> 9) - 8;
    return is_input(address) ? (input_group == configuration.input_address)
                             : (output_address == configuration.output_address);
}

void StackChannelCard::push_data(uint16_t address)
{
    assert(!is_input(address));
    const auto out_data = static_cast<uint8_t>(address & 0xff);

    data[data_pointer] = out_data;
    data_pointer = (data_pointer + 1) % data.size();
}

uint16_t StackChannelCard::get_size() const { return data.size(); }
uint8_t StackChannelCard::get_data_at(uint16_t address) const { return data[data_pointer]; }
