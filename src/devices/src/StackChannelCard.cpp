#include "StackChannelCard.h"

#include "IOCommunicator.h"
#include "Pluribus.h"

#include <emulation_core/src/ScheduledAction.h>

StackChannelCard::StackChannelCard(const StackChannelCard::Config& config)
    : change_schedule{config.change_schedule}, pluribus{config.pluribus},
      configuration(config.configuration)
{
    assert(configuration.memory_size > 0 && "The memory size must not be 0");

    initialize_io_communicator();
    initialize_terminals();
    initialize_io_card_connections();

    set_data_size();

    pluribus->t3.subscribe([this](Edge edge) { on_t3((edge)); });

    set_next_activation_time(Scheduling::unscheduled());
}

StackChannelCard::~StackChannelCard() = default;

void StackChannelCard::initialize_io_communicator()
{
    IOCommunicatorConfiguration io_communicator_config{
            .on_need_data_for_pluribus =
                    [this](uint16_t address, Scheduling::counter_type time) {
                        return pop_data_to_bus(time);
                    },
            .on_acquire_from_pluribus =
                    [this](uint16_t address, Scheduling::counter_type time) {
                        push_data_from_bus(address, time);
                    },
            .addressed_predicate = [this](uint16_t address) { return is_addressed(address); }};
    io_communicator = std::make_shared<IOCommunicator>(
            IOCommunicator::Config{.change_schedule = change_schedule,
                                   .pluribus = pluribus,
                                   .configuration = io_communicator_config});
}

void StackChannelCard::initialize_terminals()
{
    apply_pointer_address.subscribe([this](Edge edge) { on_apply_pointer_address(edge); });
    data_transfer.subscribe([this](Edge edge) { on_data_transfer(edge); });

    transfer_cycle.request(this);

    // Outputs with Peripheral
    transfer_allowed.request(this);
    output_strobe.request(this);
    output_data.request(this, Scheduling::counter_type{0});

    // Output with ???
    end_of_transfer.request(this);
}

void StackChannelCard::initialize_io_card_connections()
{
    apply_counter.subscribe([this](Edge edge) { on_apply_counter(edge); });
    apply_pointer_address.subscribe([this](Edge edge) { on_apply_pointer(edge); });

    current_pointer_address.request(this, Scheduling::counter_type{0});
}

void StackChannelCard::step() {}

void StackChannelCard::set_data_size() { data.resize(configuration.memory_size); }

void StackChannelCard::on_t3(Edge edge)
{
    const auto time = edge.time();

    // TODO: this signal is chosen randomly, could by SYNC/ could be something else
    if (is_falling(edge) && data_counter > 0 && is_low(transfer_allowed))
    {
        transfer_allowed.set(State::HIGH, time, this);
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
    set_new_pointer(data_pointer, time);

    return data[data_pointer];
}

uint8_t StackChannelCard::pop_data_to_bus(Scheduling::counter_type time)
{
    data_counter = 0;
    stop_transfer_state(time);

    return pop_data(time);
}

bool StackChannelCard::is_addressed(uint16_t address) const
{
    const uint8_t input_group = (address & 0b00111000000000) >> 9;
    const uint8_t output_address = ((address & 0b11111000000000) >> 9) - 8;
    return is_io_input_address(address) ? (input_group == configuration.input_address)
                                        : (output_address == configuration.output_address);
}

void StackChannelCard::push_data(uint8_t out_data, Scheduling::counter_type time)
{
    data[data_pointer] = out_data;
    data_pointer = (data_pointer + 1) % data.size();
    set_new_pointer(data_pointer, time);
}

void StackChannelCard::push_data_from_bus(uint16_t address, Scheduling::counter_type time)
{
    assert(!is_io_input_address(address));
    const auto out_data = static_cast<uint8_t>(address & 0xff);

    data_counter = 0;
    stop_transfer_state(time);

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
    if (is_rising(edge))
    {
        data_pointer = new_pointer_address.get_value();
        data_counter = 0;
        stop_transfer_state(edge.time());
    }
}

void StackChannelCard::on_apply_counter(Edge edge)
{
    if (is_rising(edge))
    {
        data_counter = *new_counter_value;
        if (configuration.mode == StackChannelCardConfiguration::Channel)
        {
            // TODO: this is weird and probably not what really happens.
            // The fact is that the boot ROM expects one byte more that what it asked for...
            // ... or it can be that the analysis is wrong for the moment.
            data_counter += 1;
        }
    }
}

void StackChannelCard::on_data_transfer(Edge edge)
{
    if (is_rising(edge))
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

        transfer_allowed.set(State::LOW, time, this);

        if (data_counter == 0)
        {
            // TODO: verify this and when it's lowered. It's probably more a pulse
            end_of_transfer.set(State::HIGH, time, this);
        }
        else
        {
            // TODO: verify this and when it's lowered. It's probably more a pulse
            end_of_transfer.set(State::LOW, time, this);
        }
    }
}

void StackChannelCard::on_apply_pointer(Edge edge)
{
    if (is_rising(edge))
    {
        data_pointer = *new_pointer_address;
    }
}

void StackChannelCard::stop_transfer_state(Scheduling::counter_type time)
{
    transfer_allowed.set(State::LOW, time, this);
}

void StackChannelCard::set_new_pointer(uint16_t new_pointer, Scheduling::counter_type time)
{
    data_pointer = new_pointer;
    current_pointer_address.set(new_pointer, time, this);
}

std::vector<std::shared_ptr<Schedulable>> StackChannelCard::get_sub_schedulables()
{
    return {io_communicator};
}
