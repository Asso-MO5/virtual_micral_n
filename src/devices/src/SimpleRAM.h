#ifndef MICRALN_SIMPLERAM_H
#define MICRALN_SIMPLERAM_H

#include <emulation_core/src/ConnectedData.h>
#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>

#include <cstdint>
#include <vector>

class SimpleRAM : public SchedulableImpl
{
public:
    struct InputPins
    {
        State chip_select{};
        State output_enable{};
        State write_enable{};
        uint16_t address{};
    };

    explicit SimpleRAM(size_t size);

    void connect_data_bus(std::shared_ptr<DataBus> bus);

    void step() override;

    [[nodiscard]] ConnectedData& get_data_pins();
    void signal_chip_select(Edge edge);
    void signal_output_enable(Edge edge);
    void signal_write_enable(Edge edge);
    void set_address(uint16_t address);

    uint8_t get_direct_data(uint16_t address); // Meant for UI / Debug; not for simulation
    void set_direct_data(uint16_t address,
                         uint8_t new_data); // Meant for UI / Debug; not for simulation

private:
    ConnectedData data_pins;
    InputPins input_pins{};
    std::vector<uint8_t> data;

    void set_data_when_read_selected();
};

#endif //MICRALN_SIMPLERAM_H
