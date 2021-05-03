#include "DataOnMDBusHolder.h"

#include "Pluribus.h"

DataOnMDBusHolder::DataOnMDBusHolder(Pluribus& pluribus) : pluribus{pluribus} {}

void DataOnMDBusHolder::take_bus(Scheduling::counter_type time, uint8_t data)
{
    latched_data = data;
    pluribus.data_bus_md0_7.request(this, time);
}

void DataOnMDBusHolder::place_data(Scheduling::counter_type time)
{
    pluribus.data_bus_md0_7.set(latched_data, time, this);
    is_emitting_data = true;

    pluribus.ready.request(this);
    pluribus.ready.set(State::HIGH, time, this);
}

void DataOnMDBusHolder::release_bus(Scheduling::counter_type time)
{
    pluribus.data_bus_md0_7.release(this, time);
    pluribus.ready.release(this);
    is_emitting_data = false;
}

bool DataOnMDBusHolder::is_holding_bus() const { return is_emitting_data; }
