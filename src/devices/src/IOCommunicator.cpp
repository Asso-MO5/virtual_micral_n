#include "IOCommunicator.h"

#include "DataOnMDBusHolder.h"
#include "Pluribus.h"

#include <emulation_core/src/ScheduledAction.h>

namespace
{
    const Scheduling::counter_type IO_CARD_DELAY = 20;

}

IOCommunicator::IOCommunicator(const IOCommunicator::Config& config)
    : pluribus(config.pluribus), configuration(config.configuration)
{
    output_data_holder =
            std::make_shared<DataOnMDBusHolder>(pluribus, config.change_schedule, IO_CARD_DELAY);

    pluribus->t2.subscribe([this](Edge edge) { on_t2((edge)); });

    set_next_activation_time(Scheduling::unscheduled());
}

IOCommunicator::~IOCommunicator() = default;

void IOCommunicator::step() {}

std::vector<std::shared_ptr<Schedulable>> IOCommunicator::get_sub_schedulables()
{
    return {output_data_holder};
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
                output_data_holder->place(time, data_to_send);
            }
            else
            {
                configuration.on_acquire_from_pluribus(address, time);
            }
        }
    }
}
