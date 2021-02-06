#ifndef MICRALN_SIMPLEROM_H
#define MICRALN_SIMPLEROM_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>

#include <cstdint>
#include <vector>

class SimpleROM : public SchedulableImpl
{
public:
    struct DataPins
    {
        uint8_t data{}; // Maybe no need ? (or to force typing ?)
        bool taken{};   // True if the ROM is applying output
    };

    struct InputPins
    {
        State chip_select;
        State output_enable;
        uint16_t address;
    };

    explicit SimpleROM(std::vector<uint8_t> data);

    void step() override;

    [[nodiscard]] const DataPins& get_data_pins() const;
    void signal_chip_select(Edge edge);
    void signal_output_enable(Edge edge);
    void set_address(uint16_t address);

    uint8_t get_direct_data(uint16_t address); // Meant for UI / Debug; not for simulation

private:
    DataPins data_pins;
    InputPins input_pins{};
    std::vector<uint8_t> data;

    void set_data_when_read_selected();
};

#endif //MICRALN_SIMPLEROM_H
