#include "MemoryCard.h"

#include "Pluribus.h"

#include <emulation_core/src/DataBus.h>

namespace
{
    const Scheduling::counter_type MEMORY_READ_DELAY = 200;

    Constants8008::CycleControl cycle_control_from_cc(State cc0, State cc1)
    {
        Constants8008::CycleControl cycleControl;
        if (is_low(cc0))
        {
            cycleControl = is_low(cc1) ? Constants8008::CycleControl::PCI
                                       : Constants8008::CycleControl::PCR;
        }
        else
        {
            cycleControl = is_low(cc1) ? Constants8008::CycleControl::PCC
                                       : Constants8008::CycleControl::PCW;
        }
        return cycleControl;
    }

} // namespace

MemoryCard::MemoryCard(const MemoryCard::Config& config)
    : scheduler{config.scheduler}, pluribus(config.pluribus)
{
    set_data_size(config);

    pluribus->t2.subscribe([this](Edge edge) { on_t2((edge)); });
    pluribus->t3.subscribe([this](Edge edge) { on_t3((edge)); });
}

void MemoryCard::set_data_size(const MemoryCard::Config& config)
{
    if (config.addressing_size == Card2k)
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

    pluribus->data_bus_md0_7.request(this);
    pluribus->data_bus_md0_7.set(latched_data, time, this);

    pluribus->ready.request(this);
    pluribus->ready.set(State::LOW, time, this); // Reminder: setting the logical
                                                 // not physical value, for signals.

    set_next_activation_time(Scheduling::unscheduled());
}

void MemoryCard::load_data(std::vector<uint8_t> data) {}

void MemoryCard::on_t2(Edge edge)
{

    if (is_falling(edge))
    {
        auto [address, cycle_control] = read_address_bus();
        if (cycle_control == Constants8008::CycleControl::PCI ||
            cycle_control == Constants8008::CycleControl::PCR || is_addressed(address))
        {
            // This is the end of T2, schedule the data emission
            latch_read_data(address);
            set_next_activation_time(edge.time() + MEMORY_READ_DELAY);
            scheduler.change_schedule(get_id());
        }
    }
}
void MemoryCard::on_t3(Edge edge)
{
    if (is_falling(edge))
    {
        pluribus->data_bus_md0_7.release(this);
        pluribus->ready.release(this);
    }
}

std::tuple<uint16_t, Constants8008::CycleControl> MemoryCard::read_address_bus()
{
    uint16_t address = *pluribus->address_bus_s0_s13 & 0x3fff;

    auto cc0 = *pluribus->cc0;
    auto cc1 = *pluribus->cc1;

    Constants8008::CycleControl cycleControl = cycle_control_from_cc(cc0, cc1);

    return std::make_tuple(address, cycleControl);
}

bool MemoryCard::is_addressed(uint16_t address)
{
    return true; // TODO: implement
}

void MemoryCard::latch_read_data(uint16_t address)
{
    auto address_on_card = (data.size() == 2048) ? (address & 0x7ff) : (address & 0xfff);

    latched_data = data[address_on_card];
}
