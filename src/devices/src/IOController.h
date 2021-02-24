#ifndef MICRALN_IOCONTROLLER_H
#define MICRALN_IOCONTROLLER_H

#include <emulation_core/src/ConnectedData.h>
#include <memory>

class CPU8008;
class Edge;
class DataBus;

class IOController
{
public:
    IOController(std::shared_ptr<CPU8008> cpu, std::shared_ptr<DataBus> bus);

    void signal_phase_1(const Edge& edge);
    void signal_phase_2(const Edge& edge);
    void signal_sync(const Edge& edge);

    // For debug purposes while developing the real I/O.
    void set_data_to_send(uint8_t data);
    [[nodiscard]] uint8_t get_received_data() const;

private:
    std::shared_ptr<CPU8008> cpu;
    std::shared_ptr<DataBus> bus;
    ConnectedData latched_io_data;
    uint8_t latched_io_reg_A{};
    uint8_t latched_io_reg_b{};
    uint8_t latched_cycle_control{};
    bool will_emit{};

    uint8_t data_to_send; // Debug
    uint8_t received_data; // Debug

    void read_io_information_from_cpu();
    void process_io();
};

#endif //MICRALN_IOCONTROLLER_H
