#include "IOCommunicator.h"

#include "DataOnMDBusHolder.h"
#include "Pluribus.h"

#include <emulation_core/src/ScheduledAction.h>

namespace
{
    const Scheduling::counter_type IO_CARD_DELAY = 20;

    constexpr bool is_input(uint16_t address) { return (address & 0b11000000000000) == 0; }
}

IOCommunicator::IOCommunicator(const IOCommunicator::Config& config)
    : pluribus(config.pluribus), change_schedule(config.change_schedule),
      configuration(config.configuration)
{
    output_data_holder = std::make_unique<DataOnMDBusHolder>(*pluribus);
    place_data_on_pluribus = std::make_shared<ScheduledAction>();

    pluribus->t2.subscribe([this](Edge edge) { on_t2((edge)); });
    pluribus->t3.subscribe([this](Edge edge) { on_t3((edge)); });

}

void IOCommunicator::step() {}

std::vector<std::shared_ptr<Schedulable>> IOCommunicator::get_sub_schedulables()
{
    return {place_data_on_pluribus};
}

void IOCommunicator::on_t2(Edge edge)
{
    if (is_falling(edge))
    {
        auto [address, cycle_control] = decode_address_on_bus(*pluribus);
        if ((cycle_control == Constants8008::CycleControl::PCC) &&
            configuration.addressed_predicate(address))
        {
            if (is_input(address))
            {
                // This is the end of T2, schedule the data emission
                auto data_to_send = configuration.on_need_data_for_pluribus(address);
                output_data_holder->take_bus(edge.time(),
                                             data_to_send); // This is on this class, pulling on_t3

                place_data_on_pluribus->schedule( // This is also on this class
                        [&](Scheduling::counter_type time) {
                            output_data_holder->place_data(time);
                        },
                        edge.time() + IO_CARD_DELAY,
                        change_schedule); // Needs a change_schedule
            }
            else
            {
                configuration.on_acquire_from_pluribus(address, edge.time());
            }
        }
    }
}

void IOCommunicator::on_t3(Edge edge)
{
    if (is_falling(edge) && output_data_holder->is_holding_bus())
    {
        output_data_holder->release_bus(edge.time());
    }
}
