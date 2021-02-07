#ifndef MICRALN_CONNECTEDDATA_H
#define MICRALN_CONNECTEDDATA_H

#include <memory>

class DataBus;

class ConnectedData
{
public:
    ConnectedData() = default;

    void connect(std::shared_ptr<DataBus> bus);
    void write(uint8_t data);
    [[nodiscard]] uint8_t read() const;

    void take_bus();
    void release_bus();
    [[nodiscard]] bool is_owning_bus() const;

private:
    std::shared_ptr<DataBus> data_bus;
    uint8_t local_data{};
    bool owns_bus{};
};

#endif //MICRALN_CONNECTEDDATA_H
