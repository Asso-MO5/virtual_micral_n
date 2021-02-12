#ifndef MICRALN_CPU8008_H
#define MICRALN_CPU8008_H

#include "AddressStack.h"
#include "Instructions8008.h"
#include "Constants8008.h"

#include <array>
#include <emulation_core/src/ConnectedData.h>
#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>
#include <functional>
#include <queue>

class CPU8008 : public SchedulableImpl
{
public:
    enum class CpuState : uint8_t
    {
        WAIT = 0b000,
        T3 = 0b001,
        T1 = 0b010,
        STOPPED = 0b011,
        T2 = 0b100,
        T5 = 0b101,
        T1I = 0b110,
        T4 = 0b111,
    };


    enum class Register : uint8_t
    {
        A = 0b000,
        B = 0b001,
        C = 0b010,
        D = 0b011,
        E = 0b100,
        H = 0b101,
        L = 0b110,
        Memory = 0b111,
    };

    struct OutputPins
    {
        CpuState state{CpuState::STOPPED};
        State sync{State::LOW};
    };

    struct InputPins
    {
        ::State interrupt;
        ::State ready;
        ::State phase_1;
        ::State phase_2;
        ::State vdd;
    };

    struct HiddenRegisters
    {
        uint8_t a;
        uint8_t b;
    };

    enum class Flags : uint8_t
    {
        Carry,
        Zero,
        Sign,
        Parity,
        MAX
    };

    static const size_t SCRATCH_PAD_SIZE = 7;

    struct DebugData
    {
        uint16_t pc;
        uint8_t instruction_register;
        HiddenRegisters hidden_registers;
        uint8_t registers[SCRATCH_PAD_SIZE];
    };

    ConnectedData data;

    explicit CPU8008(SignalReceiver& scheduler);

    void connect_data_bus(std::shared_ptr<DataBus> bus);

    [[nodiscard]] DebugData get_debug_data() const;

    void step() override;
    [[nodiscard]] const OutputPins& get_output_pins() const;
    [[nodiscard]] const ConnectedData& get_data_pins() const;

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
        DATA_IN
    };

    using NextEventType = std::tuple<Scheduling::counter_type, Event, uint8_t>;

private:
    OutputPins output_pins{};
    ConnectedData data_pins{};
    InputPins input_pins{};
    AddressStack address_stack;
    HiddenRegisters hidden_registers{};
    uint8_t flags[static_cast<size_t>(Flags::MAX)];
    uint8_t scratch_pad_memory[SCRATCH_PAD_SIZE]; //A, B, C, D, E, H, L registers
    uint8_t io_data_latch{};
    uint8_t instruction_register{};

    Constants8008::CycleControl cycle_control{Constants8008::CycleControl::PCI};
    uint8_t memory_cycle{}; // Can be reduced to distinguished between the second and third cycle?
    bool is_first_phase_cycle{true};
    bool interrupt_pending{};
    bool cycle_ended{};

    SignalReceiver& scheduler;
    std::priority_queue<NextEventType, std::vector<NextEventType>> next_events;
    std::function<void(Edge)> sync_callback = [](Edge) {};

    InstructionTableFor8008 instruction_table;
    InstructionTableFor8008::DecodedInstruction decoded_instruction;

    void on_signal_11_raising(Scheduling::counter_type edge_time);
    void on_signal_12_raising(Scheduling::counter_type edge_time);
    void on_signal_21_raising(Scheduling::counter_type edge_time);
    void on_signal_21_falling(Scheduling::counter_type edge_time);
    void on_signal_22_falling(Scheduling::counter_type edge_time);

    void schedule_next_event(Scheduling::counter_type edge_time);

    void update_memory_cycle();
    void execute_common_ti1_ti();
    void execute_t1i();
    void execute_t1();
    void execute_t2();
    void execute_t3();
    void checks_conditional_cycle_end(const CycleActionsFor8008::T3_Action& action);
    void execute_t4();
    void checks_cycle_end(uint8_t action);
    void execute_t5();
    void update_flags(const uint8_t& reg);
};

#endif //MICRALN_CPU8008_H
