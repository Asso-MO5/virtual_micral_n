#include "IOCard.h"

#include "DataOnMDBusHolder.h"
#include "Pluribus.h"

namespace
{
    const Scheduling::counter_type IO_CARD_DELAY = 20;

    // TODO: Duplicated from StackChannelCard
    constexpr bool is_input(uint16_t address) { return (address & 0b11000000000000) == 0; }

    // TODO: temporary dummy data waiting for real data fetch from peripheral
    const uint8_t DUMMY_DATA = 0x94;
} // namespace

IOCard::IOCard(const IOCard::Config& config)
    : scheduler{config.scheduler}, pluribus{config.pluribus}, configuration(config.configuration)
{
    output_data_holder = std::make_unique<DataOnMDBusHolder>(*pluribus);

    set_next_activation_time(Scheduling::unscheduled());

    pluribus->t2.subscribe([this](Edge edge) { on_t2((edge)); });
    pluribus->t3.subscribe([this](Edge edge) { on_t3((edge)); });

    acquire_values();
}

IOCard::~IOCard() = default;

void IOCard::acquire_values()
{
    size_t first_owned_value_to_acquire =
            configuration.mode == IOCardConfiguration::Input_32_Output_32 ? 4 : 0;
    for (auto index = first_owned_value_to_acquire; index < 8; ++index)
    {
        data_terminals[index].request(this, OwnedValue<uint8_t>::counter_type{0});
    }
}

void IOCard::step()
{
    const auto time = get_next_activation_time();
    output_data_holder->place_data(time);

    set_next_activation_time(Scheduling::unscheduled());
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
                auto data_to_send = DUMMY_DATA;
                output_data_holder->take_bus(edge.time(), data_to_send);

                set_next_activation_time(edge.time() + IO_CARD_DELAY);
                scheduler.change_schedule(get_id());
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
}
