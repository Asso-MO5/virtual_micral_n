#ifndef MICRALN_CPU8008_H
#define MICRALN_CPU8008_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>

class CPU8008 : public SchedulableImpl
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
        ::State sync; // 1 bit
    };

    struct InputPins
    {
        ::State interrupt;
        ::State ready;
        ::State phase_1;
        ::State phase_2;
        ::State vdd;
    };

    struct DataPins
    {
        uint8_t data; // Maybe no need ? (or to force typing ?)
    };

    CPU8008();

    void step() override;
    [[nodiscard]] const OutputPins& get_output_pins() const;
    [[nodiscard]] const DataPins& get_data_pins() const;

    void signal_phase_1(Edge edge, Scheduling::counter_type time);
    void signal_vdd(Edge edge, Scheduling::counter_type time);
    void signal_interrupt(Edge edge, Scheduling::counter_type time);

private:
    uint64_t clock_1_count = 0;
    OutputPins output_pins;
    DataPins data_pins;
    InputPins input_pins;
};

#endif //MICRALN_CPU8008_H
