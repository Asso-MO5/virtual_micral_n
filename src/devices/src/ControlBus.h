#ifndef MICRALN_CONTROLBUS_H
#define MICRALN_CONTROLBUS_H

#include <memory>

class CPU8008;
class SimpleROM;
class SimpleRAM;
class Edge;

class ControlBus
{
public:
    ControlBus(std::shared_ptr<CPU8008> cpu, std::shared_ptr<SimpleROM> rom,
               std::shared_ptr<SimpleRAM> ram);

    void signal_phase_1(const Edge& edge);
    void signal_phase_2(const Edge& edge);
    void signal_sync(const Edge& edge);

private:
    std::shared_ptr<CPU8008> cpu;
    std::shared_ptr<SimpleROM> rom;
    std::shared_ptr<SimpleRAM> ram;
    uint16_t latched_address{};
    uint8_t latched_cycle_control{};

    void read_address_from_cpu();
    void rom_output_enable(const Edge& edge);
    void rom_output_disable(const Edge& edge);
    void ram_output_enable(const Edge& edge);
    void ram_output_disable(const Edge& edge);
    void ram_write_enable(const Edge& edge);
    void ram_write_disable(const Edge& edge);
};

#endif //MICRALN_CONTROLBUS_H
