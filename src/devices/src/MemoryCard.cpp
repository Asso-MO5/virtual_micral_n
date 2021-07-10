#include "MemoryCard.h"

#include "DataOnMDBusHolder.h"
#include "MemoryPage.h"
#include "Pluribus.h"

#include <utility>

namespace
{
    const Scheduling::counter_type MEMORY_READ_DELAY = 200;
    const uint16_t PAGE_SIZE = 256;

}

MemoryCard::MemoryCard(const MemoryCard::Config& config)
    : change_schedule{config.change_schedule}, pluribus{config.pluribus},
      configuration{config.configuration}
{
    output_data_holder = std::make_unique<DataOnMDBusHolder>(*pluribus);

    set_next_activation_time(Scheduling::unscheduled());

    set_data_size();
    create_memory_pages();

    pluribus->t2.subscribe([this](Edge edge) { on_t2((edge)); });
    pluribus->t3.subscribe([this](Edge edge) { on_t3((edge)); });
    pluribus->t3prime.subscribe([this](Edge edge) { on_t3prime((edge)); });
}

void MemoryCard::create_memory_pages()
{
    const uint_least16_t page_count = buffer.size() / PAGE_SIZE;
    assert((PAGE_SIZE * page_count == buffer.size()) &&
           "The total size must be a multiple of the page size.");
    page_readers.reserve(page_count);
    page_writers.reserve(page_count);
    for (uint_least16_t page_index = 0; page_index < page_count; page_index += 1)
    {
        const auto start_page_address = page_index * PAGE_SIZE;
        const auto end_page_address = start_page_address + PAGE_SIZE;
        std::span<uint8_t> page_memory{begin(buffer) + start_page_address,
                                       begin(buffer) + end_page_address};
        page_readers.push_back(std::make_unique<ActiveMemoryPage>(page_memory));

        if (configuration.access_type == MemoryCardConfiguration::RAM ||
            configuration.access_type == MemoryCardConfiguration::ROM_RAM_256)
        {
            page_writers.push_back(std::make_unique<ActiveMemoryPage>(page_memory));
        }
        else
        {
            page_writers.push_back(std::make_unique<InactiveMemoryPage>());
        }
    }

    if (configuration.access_type == MemoryCardConfiguration::ROM_RAM_256)
    {
        masked_rom.resize(PAGE_SIZE);
        page_readers[0] = std::make_unique<ActiveMemoryPage>(masked_rom);
    }
}

MemoryCard::~MemoryCard() = default;

void MemoryCard::load_data(std::vector<uint8_t> data_to_load)
{
    if (masked_rom.empty())
    {
        const auto size_to_copy = std::min(data_to_load.size(), buffer.size());
        std::copy_n(begin(data_to_load), size_to_copy, begin(buffer));
    }
    else
    {
        const auto size_to_copy = std::min(data_to_load.size(), masked_rom.size());
        std::copy_n(begin(data_to_load), size_to_copy, begin(masked_rom));
    }
}

void MemoryCard::set_data_size()
{
    if (configuration.addressing_size == AddressingSize::Card2k)
    {
        buffer.resize(2048);
    }
    else
    {
        buffer.resize(4096);
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
            auto data_to_send = read_data(address);
            output_data_holder->take_bus(edge.time(), data_to_send);

            set_next_activation_time(edge.time() + MEMORY_READ_DELAY);
            change_schedule(get_id());
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
            write_data(address, data_on_bus);
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

uint8_t MemoryCard::read_data(uint16_t address) const
{
    auto [address_on_card, page_number, address_in_page] = get_local_address(address);
    return page_readers[page_number]->read(address_in_page);
}

void MemoryCard::write_data(uint16_t address, uint8_t data)
{
    auto [address_on_card, page_number, address_in_page] = get_local_address(address);
    page_writers[page_number]->write(address_in_page, data);
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

std::tuple<uint16_t, size_t, uint16_t> MemoryCard::get_local_address(uint16_t address) const
{
    assert(address >= get_start_address() && "Address is not valid for this Memory Card");
    const auto address_on_card = address - get_start_address();
    const auto page_number = address_on_card / PAGE_SIZE;
    const auto address_in_page = address_on_card - (page_number * PAGE_SIZE);

    return {address_on_card, page_number, address_in_page};
}

uint16_t MemoryCard::get_length() const { return buffer.size(); }

uint8_t MemoryCard::get_data_at(uint16_t address) const
{
    // TODO: this function is now useless
    return read_data(address);
}
std::vector<std::shared_ptr<Schedulable>> MemoryCard::get_sub_schedulables() { return {}; }
