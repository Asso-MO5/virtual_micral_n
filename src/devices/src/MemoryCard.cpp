#include "MemoryCard.h"

#include "DataOnMDBusHolder.h"
#include "Pluribus.h"

#include <utility>

namespace
{
    const Scheduling::counter_type MEMORY_READ_DELAY = 200;
} // namespace

MemoryCard::MemoryCard(const MemoryCard::Config& config)
    : scheduler{config.scheduler}, pluribus{config.pluribus}, configuration{config.configuration}
{
    output_data_holder = std::make_unique<DataOnMDBusHolder>(*pluribus);

    set_next_activation_time(Scheduling::unscheduled());

    set_data_size();

    pluribus->t2.subscribe([this](Edge edge) { on_t2((edge)); });
    pluribus->t3.subscribe([this](Edge edge) { on_t3((edge)); });
    pluribus->t3prime.subscribe([this](Edge edge) { on_t3prime((edge)); });
}

MemoryCard::~MemoryCard() = default;

void MemoryCard::load_data(std::vector<uint8_t> data_to_load)
{
    auto initial_size = data.size();
    data = std::move(data_to_load);
    data.resize(initial_size);
}

void MemoryCard::set_data_size()
{
    if (configuration.addressing_size == AddressingSize::Card2k)
    {
        data.resize(2048);
    }
    else
    {
        data.resize(4096);
    }
}

void MemoryCard::step()
{
    auto time = get_next_activation_time();
    output_data_holder->place_data(time);

    set_next_activation_time(Scheduling::unscheduled());
}

void MemoryCard::on_t2(Edge edge)
{
    if (is_falling(edge))
    {
        auto [address, cycle_control] = decode_address_on_bus(*pluribus);
        if ((cycle_control == Constants8008::CycleControl::PCI ||
             cycle_control == Constants8008::CycleControl::PCR) &&
            is_addressed(address))
        {
            // This is the end of T2, schedule the data emission
            auto data_to_send = get_data(address);
            output_data_holder->take_bus(edge.time(), data_to_send);

            set_next_activation_time(edge.time() + MEMORY_READ_DELAY);
            scheduler.change_schedule(get_id());
        }
    }
}
void MemoryCard::on_t3(Edge edge)
{
    if (is_falling(edge) && output_data_holder->is_holding_bus())
    {
        output_data_holder->release_bus(edge.time());
    }
}

void MemoryCard::on_t3prime(Edge edge)
{
    if (is_falling(edge))
    {
        auto [address, cycle_control] = decode_address_on_bus(*pluribus);
        if (cycle_control == Constants8008::CycleControl::PCW && is_addressed(address))
        {
            auto data_on_bus = pluribus->data_bus_d0_7.get_value();
            auto local_address = address - get_start_address();
            auto page_number = local_address / 512;
            if (configuration.writable_page.at(page_number))
            {
                data.at(local_address) = data_on_bus;
            }
        }
    }
}

bool MemoryCard::is_addressed(uint16_t address)
{
    bool s13 = address & 0b10000000000000;
    bool s12 = address & 0b01000000000000;
    bool s11 = address & 0b00100000000000;

    if (get_addressing_size() == AddressingSize::Card2k && s11 != configuration.selection_mask[2])
    {
        return false;
    }

    const auto& selection_mask = configuration.selection_mask;
    return s13 == selection_mask[0] && s12 == selection_mask[1];
}

uint8_t MemoryCard::get_data(uint16_t address) const
{
    auto address_on_card = (get_addressing_size() == AddressingSize::Card2k) ? (address & 0x7ff)
                                                                             : (address & 0xfff);

    return data[address_on_card];
}

MemoryCard::AddressingSize MemoryCard::get_addressing_size() const
{
    return configuration.addressing_size;
}

uint16_t MemoryCard::get_start_address() const
{
    auto& selection_mask = configuration.selection_mask;
    auto first_page_address =
            (selection_mask[0] << 13 | selection_mask[1] << 12 | selection_mask[2] << 11);
    if (get_addressing_size() == AddressingSize::Card4k)
    {
        first_page_address &= 0b11000000000000;
    }

    return first_page_address;
}

uint16_t MemoryCard::get_length() const { return data.size(); }

uint8_t MemoryCard::get_data_at(uint16_t address) const { return data.at(address); }

MemoryCardConfiguration get_rom_2k_configuration(bool s13, bool s12, bool s11)
{
    return {
            .addressing_size = MemoryCard::AddressingSize::Card2k,
            .writable_page =
                    {
                            false,
                            false,
                            false,
                            false,
                            false,
                            false,
                            false,
                            false,
                    },
            .selection_mask = {s13, s12, s11},
    };
}

MemoryCardConfiguration get_ram_2k_configuration(bool s13, bool s12, bool s11)
{
    return {
            .addressing_size = MemoryCard::AddressingSize::Card2k,
            .writable_page =
                    {
                            true,
                            true,
                            true,
                            true,
                            true,
                            true,
                            true,
                            true,
                    },
            .selection_mask = {s13, s12, s11},
    };
}
