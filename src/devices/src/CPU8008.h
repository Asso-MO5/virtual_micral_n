#ifndef MICRALN_CPU8008_H
#define MICRALN_CPU8008_H

#include <emulation_core/src/Schedulable.h>

class CPU8008 : public Schedulable
{
public:
    enum class State : uint8_t
    {
        WAIT = 0,
        T3 = 1,
        T1 = 2,
        STOPPED = 3,
        T2 = 4,
        T5 = 5,
        T1I = 6,
        T4 = 7,
    };

    struct OutputPins
    {
        State state;  // Would it be interesting to pack to 3 bits
        uint8_t sync; // 1 bit
    };

    struct InputPins
    {
        uint8_t interrupt;
        uint8_t ready;
        uint8_t phase_1;
        uint8_t phase_2;
        uint8_t vdd;
    };

    struct DataPins
    {
        uint8_t data; // Maybe no need ? (or to force typing ?)
    };

    CPU8008();

    void step() override;
    [[nodiscard]] Scheduling::counter_type get_next_activation_time() const override;
    [[nodiscard]] const OutputPins& get_output_pins() const;

private:
    OutputPins output_pins;
};

#endif //MICRALN_CPU8008_H
