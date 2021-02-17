#ifndef MICRALN_CONTROLBUS_H
#define MICRALN_CONTROLBUS_H

#include <memory>

class CPU8008;
class SimpleROM;
class Edge;

class ControlBus
{
public:
    ControlBus(std::shared_ptr<CPU8008> cpu, std::shared_ptr<SimpleROM> rom);

    void signal_phase_1(const Edge& edge);
    void signal_phase_2(const Edge& edge);
    void signal_sync(const Edge& edge);

private:
    std::shared_ptr<CPU8008> cpu;
    std::shared_ptr<SimpleROM> rom;
    uint16_t rom_address_bus{};
    uint8_t latched_cycle_control{};

    void read_address_from_cpu();
    void rom_output_enable(const Edge& edge);
    void rom_output_disable(const Edge& edge);
};

#endif //MICRALN_CONTROLBUS_H
