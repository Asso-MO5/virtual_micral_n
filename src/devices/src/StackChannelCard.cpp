#include "StackChannelCard.h"

#include "DataOnMDBusHolder.h"
#include "IOCard.h"
#include "Pluribus.h"

namespace
{
    const Scheduling::counter_type STACK_MEMORY_READ_DELAY = 150;

    constexpr bool is_input(uint16_t address) { return (address & 0b11000000000000) == 0; }
} // namespace

StackChannelCard::StackChannelCard(const StackChannelCard::Config& config)
    : scheduler{config.scheduler}, pluribus{config.pluribus}, configuration(config.configuration)
{
    assert(configuration.memory_size > 0 && "The memory size must not be 0");

    output_data_holder = std::make_unique<DataOnMDBusHolder>(*pluribus);

    initialize_terminals();
    initialize_io_card_connections();

    set_next_activation_time(Scheduling::unscheduled());
    set_data_size();

    pluribus->t2.subscribe([this](Edge edge) { on_t2((edge)); });
    pluribus->t3.subscribe([this](Edge edge) { on_t3((edge)); });
}

StackChannelCard::~StackChannelCard() = default;

void StackChannelCard::initialize_terminals()
{
    apply_pointer_address.subscribe([this](Edge edge) { on_apply_pointer_address(edge); });
    data_transfer.subscribe([this](Edge edge) { on_data_transfer(edge); });

    // Just needs to read the value? Or are there side effects like resetting the counter?
    // direction.subscribe([this](Edge edge) { on_direction_change(edge); });

    // Output with I/O
    current_pointer_address.request(this, Scheduling::counter_type{0});

    // Outputs with Peripheral
    in_transfer.request(this);
    output_strobe.request(this);
    output_data.request(this, Scheduling::counter_type{0});
}

void StackChannelCard::initialize_io_card_connections()
{
    if (configuration.io_card)
    {
        configuration.io_card->ack_terminals[configuration.new_counter_terminal].subscribe(
                [this](Edge edge) { on_apply_counter(edge); });
    }
}

void StackChannelCard::step()
{
    // Note: currently, the only step action is to send data on md0-md7.
    // If other actions happen, this will have to be changed.
    const auto time = get_next_activation_time();
    output_data_holder->place_data(time);

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
            const auto time = edge.time();
            if (is_input(address))
            {
                // This is the end of T2, schedule the data emission
                auto data_to_send = pop_data_to_bus(time);

                output_data_holder->take_bus(edge.time(), data_to_send);

                set_next_activation_time(edge.time() + STACK_MEMORY_READ_DELAY);
                scheduler.change_schedule(get_id());
            }
            else
            {
                push_data_from_bus(address, time);
            }
        }
    }
}
void StackChannelCard::on_t3(Edge edge)
{
    if (is_falling(edge) && output_data_holder->is_holding_bus())
    {
        output_data_holder->release_bus(edge.time());
    }
}

uint8_t StackChannelCard::pop_data(Scheduling::counter_type time)
{
    if (data_pointer > 0)
    {
        data_pointer -= 1;
    }
    else
    {
        data_pointer = data.size() - 1;
    }
    current_pointer_address.set(data_pointer, time, this);

    return data[data_pointer];
}

uint8_t StackChannelCard::pop_data_to_bus(Scheduling::counter_type time)
{
    data_counter = 0;
    return pop_data(time);
}

bool StackChannelCard::is_addressed(uint16_t address) const
{
    const uint8_t input_group = (address & 0b00111000000000) >> 9;
    const uint8_t output_address = ((address & 0b11111000000000) >> 9) - 8;
    return is_input(address) ? (input_group == configuration.input_address)
                             : (output_address == configuration.output_address);
}

void StackChannelCard::push_data(uint8_t out_data, Scheduling::counter_type time)
{
    data[data_pointer] = out_data;
    data_pointer = (data_pointer + 1) % data.size();
    current_pointer_address.set(data_pointer, time, this);
}

void StackChannelCard::push_data_from_bus(uint16_t address, Scheduling::counter_type time)
{
    assert(!is_input(address));
    const auto out_data = static_cast<uint8_t>(address & 0xff);

    data_counter = 0;
    push_data(out_data, time);
}

uint8_t StackChannelCard::get_data_at(uint16_t address) const { return data[address]; }
StackChannelCard::DebugData StackChannelCard::get_debug_data() const
{
    return {
            static_cast<uint16_t>(data.size()),
            data_pointer,
            data_counter,
    };
}

void StackChannelCard::on_apply_pointer_address(Edge edge)
{
    data_pointer = new_pointer_address.get_value();
    data_counter = 0;
}

void StackChannelCard::on_apply_counter(Edge edge)
{
    if (is_rising(edge))
    {
        data_counter = *configuration.io_card->data_terminals[configuration.new_counter_terminal];
    }
}

void StackChannelCard::on_data_transfer(Edge edge)
{
    const auto time = edge.time();

    if (data_counter == 0)
    {
        return;
    }

    data_counter -= 1;

    if (*direction == State::HIGH)
    {
        // Writing to the Stack
        push_data(input_data.get_value(), time);
    }
    else
    {
        // Reading from the Stack
        // TODO: To be implemented
    }

    if (data_counter == 0)
    {
        // TODO: raise end of transfer
    }
}
