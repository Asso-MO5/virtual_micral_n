#ifndef MICRALN_CPU8008_H
#define MICRALN_CPU8008_H

#include "AddressStack.h"

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/OwnedSignal.h>
#include <emulation_core/src/OwnedValue.h>
#include <emulation_core/src/Schedulable.h>
#include <i8008/src/Constants8008.h>
#include <i8008/src/Instructions8008.h>

#include <array>
#include <functional>
#include <queue>

class CPU8008 : public SchedulableImpl
{
public:
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
        OwnedValue<Constants8008::CpuState> state{Constants8008::CpuState::STOPPED};
        OwnedSignal sync;
    };

    struct InputPins
    {
        OwnedSignal interrupt;
        OwnedSignal ready{State::HIGH};
        // phase_1{}; // These are not used, but could be OwnedSignals.
        // phase_2{}; // No necessity at the moment.
        OwnedSignal vdd{};
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
        uint8_t instruction_register{};
        HiddenRegisters hidden_registers{};
        uint8_t registers[SCRATCH_PAD_SIZE]{};
        uint8_t flags[static_cast<size_t>(Flags::MAX)];
        uint16_t latest_emitted_pci;
        AddressStack::DebugData address_stack;
        InstructionTableFor8008::DecodedInstruction decoded_instruction;
        Constants8008::CycleControl cycle_control;
    };

    explicit CPU8008(Scheduling::change_schedule_cb scheduler);

    [[nodiscard]] DebugData get_debug_data() const;

    std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables() override;

    void step() override;

    void signal_phase_1(Edge edge);
    void signal_phase_2(Edge edge);

    void register_sync_trigger(std::function<void(Edge)> callback);

    using state_callback_type = std::function<void(Constants8008::CpuState, Constants8008::CpuState,
                                                   Scheduling::counter_type)>;
    void register_state_change(const state_callback_type& callback);

    enum Event
    {
        SYNC,
        STATE,
        DATA_OUT,
        DATA_IN
    };

    using NextEventType = std::tuple<Scheduling::counter_type, Event, uint8_t>;

    OwnedValue<uint8_t> data_pins{};
    OutputPins output_pins{};
    InputPins input_pins{};

private:
    AddressStack address_stack;
    HiddenRegisters hidden_registers{};
    uint8_t flags[static_cast<size_t>(Flags::MAX)]{};
    uint8_t scratch_pad_memory[SCRATCH_PAD_SIZE]; //A, B, C, D, E, H, L registers
    uint8_t io_data_latch{};
    uint8_t instruction_register{};
    uint16_t latest_emitted_pci{};

    Constants8008::CycleControl cycle_control{Constants8008::CycleControl::PCI};
    Constants8008::CycleControl next_cycle_control{Constants8008::CycleControl::PCI};

    uint8_t memory_cycle{}; // Can be reduced to distinguished between the second and third cycle?
    bool is_first_phase_cycle{true};
    bool interrupt_pending{};
    bool cycle_ended{};

    Scheduling::change_schedule_cb change_schedule;
    std::priority_queue<NextEventType, std::vector<NextEventType>> next_events;

    InstructionTableFor8008 instruction_table;
    InstructionTableFor8008::DecodedInstruction decoded_instruction;

    void on_interrupt(Edge edge);
    void on_vdd(Edge edge);

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

    void ends_cycle(Constants8008::CycleControl new_cycle_control);
    [[nodiscard]] bool is_instruction_complete() const;
    void enter_interrupt(Scheduling::counter_type edge_time);
    void schedule_change_cpu_state(Scheduling::counter_type edge_time);
};

#endif //MICRALN_CPU8008_H
