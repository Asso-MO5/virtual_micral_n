#ifndef MICRALN_DATABUS_H
#define MICRALN_DATABUS_H

#include <unordered_set>
#include <string>

struct bus_error : public std::exception
{
    explicit bus_error(const char* message) : msg(message) {}
    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

protected:
    std::string msg;
};

class ConnectedData;

class DataBus
{
public:
    DataBus() = default;

    [[nodiscard]] uint8_t read() const;

    void write(uint8_t data, ConnectedData* connected_data);
    void request(ConnectedData* connected_data);
    void release(ConnectedData* connected_data);
    void connect(ConnectedData* connected_data);

private:
    ConnectedData* current_owner = nullptr;
    std::unordered_set<ConnectedData*> all_connected_data;

    uint8_t data_on_bus{};
};

#endif //MICRALN_DATABUS_H
