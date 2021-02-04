#ifndef MICRALN_CPU8008_H
#define MICRALN_CPU8008_H

#include "AddressStack.h"

#include <array>
#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>
#include <functional>
#include <queue>

class CPU8008 : public SchedulableImpl
{
public:
    enum class CpuState : uint8_t
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
        CpuState state; // Would it be interesting to pack to 3 bits
        ::State sync;   // 1 bit
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

    CPU8008(SignalReceiver & scheduler);

    void step() override;
    [[nodiscard]] const OutputPins& get_output_pins() const;
    [[nodiscard]] const DataPins& get_data_pins() const;

    void signal_phase_1(Edge edge);
    void signal_phase_2(Edge edge);
    void signal_vdd(Edge edge);
    void signal_interrupt(Edge edge);

    void register_sync_trigger(std::function<void(Edge)> callback);

    enum Event
    {
        SYNC,
        STATE,
        DATA_OUT,
        DATA_INT
    };
    using NextEventType = std::tuple<Scheduling::counter_type, Event, uint8_t>;

private:
    SignalReceiver & scheduler;
    OutputPins output_pins{};
    DataPins data_pins{};
    InputPins input_pins{};

    AddressStack address_stack;
    CpuState next_state;
    bool is_first_phase_cycle; // Complete cycle

    std::priority_queue<NextEventType, std::vector<NextEventType>> next_events;

    std::function<void(Edge)> sync_callback = [](Edge) {};
};

#endif //MICRALN_CPU8008_H
