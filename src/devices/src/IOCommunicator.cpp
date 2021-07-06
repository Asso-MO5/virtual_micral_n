#include "IOCommunicator.h"

#include "DataOnMDBusHolder.h"
#include "Pluribus.h"

#include <emulation_core/src/ScheduledAction.h>

namespace
{
    const Scheduling::counter_type IO_CARD_DELAY = 20;

}

IOCommunicator::IOCommunicator(const IOCommunicator::Config& config)
    : pluribus(config.pluribus), change_schedule(config.change_schedule),
      configuration(config.configuration)
{
    output_data_holder = std::make_unique<DataOnMDBusHolder>(*pluribus);
    place_data_on_pluribus = std::make_shared<ScheduledAction>();

    pluribus->t2.subscribe([this](Edge edge) { on_t2((edge)); });
    pluribus->t3.subscribe([this](Edge edge) { on_t3((edge)); });

    set_next_activation_time(Scheduling::unscheduled());
}

IOCommunicator::~IOCommunicator() = default;

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
            const auto time = edge.time();
            if (is_io_input_address(address))
            {
                // This is the end of T2, schedule the data emission
                auto data_to_send = configuration.on_need_data_for_pluribus(address, time);
                output_data_holder->take_bus(time, data_to_send);
                place_data_on_pluribus->schedule(
                        [&](Scheduling::counter_type time) {
                            output_data_holder->place_data(time);
                        },
                        time + IO_CARD_DELAY, change_schedule);
            }
            else
            {
                configuration.on_acquire_from_pluribus(address, time);
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
