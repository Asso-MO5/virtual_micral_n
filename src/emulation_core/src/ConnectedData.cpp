#include "ConnectedData.h"

#include "DataBus.h"

void ConnectedData::connect(std::shared_ptr<DataBus> bus)
{
    data_bus = std::move(bus);
    data_bus->connect(this);
}

void ConnectedData::write(uint8_t data)
{
    local_data = data;
    if (data_bus && owns_bus)
    {
        data_bus->write(data, this);
    }
}

uint8_t ConnectedData::read() const { return local_data; }

void ConnectedData::take_bus()
{
    if (data_bus)
    {
        data_bus->request(this);
    }
    owns_bus = true;
}

void ConnectedData::release_bus()
{
    if (data_bus)
    {
        data_bus->release(this);
    }
    owns_bus = false;
}
