#include "IOCard.h"
#include "DataOnMDBusHolder.h"
#include "Pluribus.h"

namespace
{
    const Scheduling::counter_type IO_CARD_DELAY = 20;
    const Scheduling::counter_type ACK_APPLIED_PERIOD = 200;

    // TODO: Duplicated from StackChannelCard
    constexpr bool is_input(uint16_t address) { return (address & 0b11000000000000) == 0; }
}

IOCard::IOCard(const IOCard::Config& config)
    : change_schedule{config.change_schedule}, pluribus{config.pluribus},
      configuration{config.configuration}
{
    output_data_holder = std::make_unique<DataOnMDBusHolder>(*pluribus);

    next_time_to_place_data = Scheduling::unscheduled();

    pluribus->t2.subscribe([this](Edge edge) { on_t2((edge)); });
    pluribus->t3.subscribe([this](Edge edge) { on_t3((edge)); });

    switch (configuration.mode)
    {
        case IOCardConfiguration::Input_32_Output_32:
            first_owned_terminal = 4;
            break;
        case IOCardConfiguration::Input_64:
            first_owned_terminal = IOCardConstants::TERMINAL_COUNT;
            break;
        case IOCardConfiguration::Output_64:
            first_owned_terminal = 0;
            break;
    }

    initialize_terminals();
    update_next_activation_time();
}

IOCard::~IOCard() = default;

void IOCard::initialize_terminals()
{
    for (size_t index = 0; index < first_owned_terminal; ++index)
    {
        ack_terminals[index].subscribe([index, this](Edge edge) { on_input_signal(index, edge); });
    }

    for (size_t index = first_owned_terminal; index < IOCardConstants::TERMINAL_COUNT; ++index)
    {
        data_terminals[index].request(this, OwnedValue<uint8_t>::counter_type{0});
        ack_terminals[index].request(this);
        next_time_for_ack_low[index] = Scheduling::unscheduled();
    }
}

void IOCard::step()
{
    const auto time = get_next_activation_time();

    if (time == next_time_to_place_data)
    {
        output_data_holder->place_data(time);
        next_time_to_place_data = Scheduling::unscheduled();
    }

    for (auto index = first_owned_terminal; index < IOCardConstants::TERMINAL_COUNT; ++index)
    {
        if (time == next_time_for_ack_low[index])
        {
            ack_terminals[index].set(State::LOW, time, this);
            next_time_for_ack_low[index] = Scheduling::unscheduled();
        }
    }

    update_next_activation_time();
}

void IOCard::on_t2(Edge edge)
{
    // TODO: Duplicated from StackChannelCard
    if (is_falling(edge))
    {
        auto [address, cycle_control] = decode_address_on_bus(*pluribus);
        if ((cycle_control == Constants8008::CycleControl::PCC) && is_addressed(address))
        {
            if (is_input(address))
            {
                // This is the end of T2, schedule the data emission
                auto data_to_send = get_from_peripheral(address);
                output_data_holder->take_bus(edge.time(), data_to_send);

                next_time_to_place_data = edge.time() + IO_CARD_DELAY;
                set_next_activation_time(edge.time() + IO_CARD_DELAY);
                update_next_activation_time();
            }
            else
            {
                send_to_peripheral(address, edge.time());
            }
        }
    }
}

void IOCard::on_t3(Edge edge)
{
    // TODO: Duplicated from StackChannelCard
    if (is_falling(edge) && output_data_holder->is_holding_bus())
    {
        output_data_holder->release_bus(edge.time());
    }
}

void IOCard::update_next_activation_time()
{
    auto next_activation_time = next_time_to_place_data;
    for (auto index = first_owned_terminal; index < IOCardConstants::TERMINAL_COUNT; ++index)
    {
        next_activation_time = std::min(next_activation_time, next_time_for_ack_low[index]);
    }

    set_next_activation_time(next_activation_time);
    change_schedule(get_id());
}

namespace
{
    bool match_for_32_32(uint16_t address, uint8_t selection)
    {
        if (is_input(address))
        {
            uint8_t group = ((address >> 8) & 0b00001110) >> 1;
            if (group != 0)
            {
                return false;
            }

            uint8_t s2 = ((address >> 2)) & 0b00000001;
            return s2 == (selection & 0b00000001);
        }
        else
        {
            uint8_t s13_to_s11 = ((address >> 11) << 5) & 0b11100000;
            return s13_to_s11 == (selection & 0b11100000);
        }
    }

    bool match_for_64_inputs(uint16_t address, uint8_t selection)
    {
        uint8_t s11_to_s9 = ((address >> 9) << 5) & 0b11100000;
        uint8_t s7_to_s3 = (address >> 3) & 0b00011111;
        return (s11_to_s9 | s7_to_s3) == selection;
    }

    bool match_for_64_outputs(uint16_t address, uint8_t selection)
    {
        uint8_t s13_to_s12 = ((address >> 12) << 6) & 0b11000000;
        return (s13_to_s12 & 0b11000000) == selection;
    }

} // namespace

bool IOCard::is_addressed(uint16_t address) const
{
    switch (configuration.mode)
    {
        case IOCardConfiguration::Input_32_Output_32: {
            return match_for_32_32(address, configuration.address_selection);
        }
        case IOCardConfiguration::Input_64: {
            return match_for_64_inputs(address, configuration.address_selection);
        }
        case IOCardConfiguration::Output_64: {
            return match_for_64_outputs(address, configuration.address_selection);
        }
    }
    return false;
}

uint8_t IOCard::address_to_output_number(uint16_t address) const
{
    assert((configuration.mode == IOCardConfiguration::Input_32_Output_32 ||
            configuration.mode == IOCardConfiguration::Output_64) &&
           "Output makes sense only for a card supporting Output.");

    if (configuration.mode == IOCardConfiguration::Input_32_Output_32)
    {
        uint8_t s9_to_s10 = (address >> 9) & 0b11;
        return s9_to_s10;
    }
    else
    {
        assert(configuration.mode == IOCardConfiguration::Output_64);
        uint8_t s9_to_s11 = (address >> 9) & 0b111;
        return s9_to_s11;
    }
}

void IOCard::send_to_peripheral(uint16_t address, Scheduling::counter_type time)
{
    using namespace std;
    // The data to send in on the lower part of the address
    const uint8_t data = address & 0xff;
    uint8_t output_number = address_to_output_number(address);

    assert(((configuration.mode == IOCardConfiguration::Input_32_Output_32 && output_number < 4) ||
            (configuration.mode == IOCardConfiguration::Output_64 && output_number < 8)) &&
           "The output address doesn't match the card type. It shouldn't have been "
           "addressed.");

    if (configuration.mode == IOCardConfiguration::Input_32_Output_32)
    {
        output_number += 4; // First 4 values are for inputs.
    }

    data_terminals[output_number].set(data, time, this);
    ack_terminals[output_number].set(State::HIGH, time, this);

    next_time_for_ack_low[output_number] = time + ACK_APPLIED_PERIOD;

    update_next_activation_time();
}

uint8_t IOCard::address_to_input_number(uint16_t address) const
{
    assert((configuration.mode == IOCardConfiguration::Input_32_Output_32 ||
            configuration.mode == IOCardConfiguration::Input_64) &&
           "Input makes sense only for a card supporting Input.");

    uint8_t s0_to_s2 = address & 0b00000111;

    if (configuration.mode == IOCardConfiguration::Input_32_Output_32 &&
        (configuration.address_selection & 0b1))
    {
        s0_to_s2 -= 4;
    }

    return s0_to_s2;
}

uint8_t IOCard::get_from_peripheral(uint16_t address)
{
    const uint8_t input_number = address_to_input_number(address);
    assert((input_number < first_owned_terminal) && "Cannot fetch data on an output channel.");

    const uint8_t data = latched_input_data[input_number];

    return data;
}

void IOCard::on_input_signal(uint8_t signal_index, Edge edge)
{
    assert((signal_index < first_owned_terminal) && "Cannot receive data on an output channel.");

    auto data = data_terminals[signal_index].get_value();
    latched_input_data[signal_index] = data;
}

std::vector<std::shared_ptr<Schedulable>> IOCard::get_sub_schedulables() { return {}; }
