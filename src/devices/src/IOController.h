#ifndef MICRALN_IOCONTROLLER_H
#define MICRALN_IOCONTROLLER_H

#include <memory>

class CPU8008;
struct Edge;
class Pluribus;

class IOController
{
public:
    IOController(const CPU8008& cpu, std::shared_ptr<Pluribus> pluribus);

    void signal_phase_1(const Edge& edge);
    void signal_phase_2(const Edge& edge);
    void signal_sync(const Edge& edge);

    // For debug purposes while developing the real I/O.
    void set_data_to_send(uint8_t data);
    [[nodiscard]] uint8_t get_received_data() const;

private:
    const CPU8008& cpu;
    std::shared_ptr<Pluribus> pluribus;
    uint8_t latched_io_reg_A{};
    uint8_t latched_io_reg_b{};
    uint8_t latched_cycle_control{};
    bool will_emit{};

    uint8_t data_to_send{}; // Debug
    uint8_t received_data{}; // Debug

    void read_io_information_from_cpu();
    void process_io();
};

#endif //MICRALN_IOCONTROLLER_H
