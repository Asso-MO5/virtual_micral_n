#include "DataOnMDBusHolder.h"

#include "Pluribus.h"

#include <emulation_core/src/ScheduledAction.h>

#include <utility>

DataOnMDBusHolder::DataOnMDBusHolder(const std::shared_ptr<Pluribus>& pluribus,
                                     Scheduling::change_schedule_cb change_schedule,
                                     Scheduling::counter_type delay)
    : pluribus{pluribus}, change_schedule{std::move(change_schedule)}, delay{delay}
{
    place_data_on_pluribus = std::make_shared<ScheduledAction>();

    pluribus->t3.subscribe([this](Edge edge) { on_t3((edge)); });
    pluribus->sub.subscribe([this](Edge edge) { on_sub((edge)); });

    set_next_activation_time(Scheduling::unscheduled());
}

void DataOnMDBusHolder::place(Scheduling::counter_type time, uint8_t data)
{
    latched_data = data;
    has_data_to_send = true;
    if (is_low(pluribus->sub))
    {
        take_bus(time);
    }
}

void DataOnMDBusHolder::take_bus(Scheduling::counter_type time)
{
    pluribus->data_bus_md0_7.request(this, time);
    owns_bus = true;

    place_data_on_pluribus->schedule([&](Scheduling::counter_type time) { place_data(time); },
                                     time + delay, change_schedule);
}

void DataOnMDBusHolder::place_data(Scheduling::counter_type time)
{
    if (owns_bus)
    {
        pluribus->data_bus_md0_7.set(latched_data, time, this);

        pluribus->ready.request(this);
        pluribus->ready.set(State::HIGH, time, this);
    }
}

void DataOnMDBusHolder::release_bus(Scheduling::counter_type time)
{
    pluribus->data_bus_md0_7.set(0, time, this);
    pluribus->data_bus_md0_7.release(this, time);
    pluribus->ready.release(this);
    owns_bus = false;
}

void DataOnMDBusHolder::on_t3(Edge edge)
{
    if (is_falling(edge) && has_data_to_send)
    {
        if (owns_bus)
        {
            release_bus(edge.time());
        }
        has_data_to_send = false;
    }
}

void DataOnMDBusHolder::on_sub(Edge edge)
{
    if (is_rising(edge) && owns_bus)
    {
        release_bus(edge.time());
    }
    else
    {
        if (has_data_to_send)
        {
            take_bus(edge.time());
        }
    }
}

void DataOnMDBusHolder::step() {}

std::vector<std::shared_ptr<Schedulable>> DataOnMDBusHolder::get_sub_schedulables()
{
    return {place_data_on_pluribus};
}
