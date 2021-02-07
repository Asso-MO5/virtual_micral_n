#include "DataBus.h"

#include "ConnectedData.h"

#include <cassert>

uint8_t DataBus::read() const { return data_on_bus; }

void DataBus::write(uint8_t data, ConnectedData* connected_data)
{
    assert(connected_data == current_owner && "Cannot write if not owning the bus");
    if (connected_data == current_owner)
    {
        data_on_bus = data;

        for (auto& connected : all_connected_data)
        {
            if (connected != connected_data)
            {
                connected->write(data);
            }
        }
    }
}

void DataBus::request(ConnectedData* connected_data)
{
    if (current_owner == nullptr)
    {
        current_owner = connected_data;
    }
    else
    {
        throw bus_error("Bus was requested but is not free.");
    }
}

void DataBus::release(ConnectedData* connected_data)
{
    if (current_owner == connected_data)
    {
        current_owner = nullptr;
    }
    else
    {
        throw bus_error("Bus was released but not taken by this data.");
    }
}

void DataBus::connect(ConnectedData* connected_data) { all_connected_data.insert(connected_data); }
