#include "CPU8008.h"
#include <emulation_core/src/Edge.h>

#include <cstring>
#include <utility>

using namespace Constants8008;

namespace
{
    const Scheduling::counter_type BOOT_UP_TIME = 16'000;

    namespace Timings
    {
        using namespace Scheduling;
        //const counter_type MIN_CLOCK_PERIOD = 2000;
        //const counter_type MAX_CLOCK_PERIOD = 3000;
        const counter_type PULSE_WIDTH_PHASE_1 = 700;
        const counter_type PULSE_WIDTH_PHASE_2 = 550;
        //const counter_type FALLING_1_TO_FALLING_2_EDGE_MIN = 900;
        //const counter_type FALLING_1_TO_FALLING_2_EDGE_MAX = 1100;
        const counter_type DELAY_FROM_PHASE_1_TO_2 = 200;
        //const counter_type DELAY_FROM_PHASE_2_TO_1 = 400;
        const counter_type DATA_OUT_HOLD_TIME = 100;
        const counter_type DATA_IN_HOLD_TIME = 100;
    } // namespace Timings

} // namespace

bool operator<(const CPU8008::NextEventType& a, const CPU8008::NextEventType& b)
{
    return std::get<0>(a) > std::get<0>(b);
}

CPU8008::CPU8008(SignalReceiver& scheduler) : scheduler(scheduler)
{
    output_pins.sync.request(this);
    output_pins.state.request(this);
}

void CPU8008::connect_data_bus(std::shared_ptr<DataBus> bus) { data_pins.connect(std::move(bus)); }

void CPU8008::step()
{
    assert((input_pins.vdd == ::State::HIGH) && "CPU without power should not be scheduled");

    if (next_events.empty())
    {
        set_next_activation_time(Scheduling::unscheduled());
        return;
    }

    auto [time, event, param] = next_events.top();
    next_events.pop();

    switch (event)
    {
        case SYNC: {
            auto sync_edge =
                    param ? Edge{Edge::Front::RISING, time} : Edge{Edge::Front::FALLING, time};
            output_pins.sync.apply(sync_edge, this);
            // TODO : Constraint :  tSD max (.70) after ø21 FALL
            break;
        }
        case STATE: {
            // TODO: Constraint : tS2 max (1.0) after ø11 RAISE (for T1/TI1)
            // TODO: Constraint : tS1 max (1.1) after ø11 RAISE (for others)
            output_pins.state.set(static_cast<CpuState>(param), time, this);

            switch (*output_pins.state)
            {
                case CpuState::T1I:
                    cycle_ended = false;
                    execute_t1i();
                    break;
                case CpuState::T1:
                    cycle_ended = false;
                    execute_t1();
                    break;
                case CpuState::T2:
                    execute_t2();
                    break;
                case CpuState::WAIT:
                    break;
                case CpuState::T3:
                    if (cycle_control == Constants8008::CycleControl::PCI ||
                        cycle_control == Constants8008::CycleControl::PCR ||
                        cycle_control == Constants8008::CycleControl::PCC)
                    {
                        // Too early to execute T3. The data is not yet present
                        // on the data bus.
                        // It will be executed on the DATA_IN signal.
                    }
                    else
                    {
                        // On a PCW Cycle, the execution is now, as there's no DATA IN
                        // issued, and no dependency on data presence on the BUS.
                        execute_t3();
                    }
                    break;
                case CpuState::STOPPED:
                    break;
                case CpuState::T4:
                    execute_t4();
                    break;
                case CpuState::T5:
                    execute_t5();
                    break;
            }
            break;
        }
        case DATA_OUT:
            // TODO: Constraint : tDD max (1.0) after ø11 FALLING
            // TODO: Constraint : tOH min (.10) after ø22 FALLING

            if (param)
            {
                uint8_t data_to_send = io_data_latch;

                if (*output_pins.state == CpuState::T2)
                {
                    data_to_send &= 0b00111111;
                    data_to_send |= static_cast<uint8_t>(cycle_control);
                }

                data_pins.take_bus();
                data_pins.write(data_to_send);
            }
            else
            {
                data_pins.release_bus();
            }
            break;
        case DATA_IN:
            execute_t3();
            break;
    }

    if (next_events.empty())
    {
        set_next_activation_time(Scheduling::unscheduled());
    }
    else
    {
        auto& next_event = next_events.top();
        set_next_activation_time(std::get<0>(next_event));
    }
}

const CPU8008::OutputPins& CPU8008::get_output_pins() const { return output_pins; }
const ConnectedData& CPU8008::get_data_pins() const { return data_pins; }

void CPU8008::on_signal_11_raising(Scheduling::counter_type edge_time)
{
    next_events.push(std::make_tuple(edge_time + 20, SYNC, 1));

    switch (*output_pins.state)
    {
        case CpuState::STOPPED:
            if (interrupt_pending)
            {
                interrupt(edge_time);
            }
            break;
        case CpuState::WAIT:
            if (input_pins.ready == State{State::HIGH})
            {
                next_events.push(
                        std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T3)));
            }
            break;
        case CpuState::T1:
        case CpuState::T1I:
            next_events.push(
                    std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T2)));
            break;
        case CpuState::T2:
            if (input_pins.ready == State{State::HIGH})
            {
                next_events.push(
                        std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T3)));
            }
            else
            {
                next_events.push(
                        std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::WAIT)));
            }
            break;
        case CpuState::T3:
            if (cycle_ended)
            {
                if (decoded_instruction.instruction->name == InstructionNameFor8008::HLT)
                {
                    cycle_control = Constants8008::CycleControl::PCI;

                    // Internally the 8008 stays in the T3 state.
                    // For emulation purposes, it goes to a « fake » STOPPED state.
                    next_events.push(std::make_tuple(edge_time + 25, STATE,
                                                     static_cast<int>(CpuState::STOPPED)));
                }
                else
                {
                    // TODO: Instruction Jammed goes to TI1?
                    if (is_instruction_complete() && interrupt_pending)
                    {
                        interrupt(edge_time);
                    }
                    else
                    {
                        next_events.push(std::make_tuple(edge_time + 25, STATE,
                                                         static_cast<int>(CpuState::T1)));
                    }
                    cycle_control = next_cycle_control;
                }
            }
            else
            {
                next_events.push(
                        std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T4)));
            }
            break;
        case CpuState::T4:
            if (cycle_ended)
            {
                // TODO: Instruction Jammed goes to TI1?
                if (is_instruction_complete() && interrupt_pending)
                {
                    interrupt(edge_time);
                }
                else
                {
                    next_events.push(
                            std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T1)));
                }
                cycle_control = next_cycle_control;
            }
            else
            {
                next_events.push(
                        std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T5)));
            }
            break;
        case CpuState::T5:
            assert(cycle_ended);
            if (interrupt_pending)
            {
                interrupt(edge_time);
            }
            else
            {
                next_events.push(
                        std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T1)));
            }
            cycle_control = CycleControl::PCI;

            break;
    }
}

void CPU8008::on_signal_12_raising(Scheduling::counter_type edge_time)
{
    next_events.push(std::make_tuple(edge_time + 20, SYNC, 0));
}

void CPU8008::on_signal_21_raising(Scheduling::counter_type edge_time)
{
    switch (*output_pins.state)
    {
        case CpuState::WAIT:
        case CpuState::STOPPED:
            break;
        case CpuState::T1I:
        case CpuState::T1:
        case CpuState::T2:
            next_events.push(std::make_tuple(edge_time + 20, DATA_OUT, 1));
            break;
        case CpuState::T3:
            if (cycle_control == Constants8008::CycleControl::PCW)
            {
                next_events.push(std::make_tuple(edge_time + 20, DATA_OUT, 1));
            }
            break;
        case CpuState::T4:
            if (cycle_control == Constants8008::CycleControl::PCC)
            {
                next_events.push(std::make_tuple(edge_time + 20, DATA_OUT, 1));
            }
            break;
        case CpuState::T5:
            break;
    }
}

void CPU8008::on_signal_21_falling(Scheduling::counter_type edge_time)
{
    if (*output_pins.state == CpuState::T3 && (cycle_control == Constants8008::CycleControl::PCI ||
                                               cycle_control == Constants8008::CycleControl::PCR ||
                                               cycle_control == Constants8008::CycleControl::PCC))
    {
        next_events.push(std::make_tuple(edge_time + Timings::DATA_IN_HOLD_TIME, DATA_IN, 1));
    }
}

void CPU8008::on_signal_22_falling(Scheduling::counter_type edge_time)
{
    switch (*output_pins.state)
    {
        case CpuState::WAIT:
        case CpuState::STOPPED:
            break;
        case CpuState::T1I:
        case CpuState::T1:
        case CpuState::T2:
            next_events.push(
                    std::make_tuple(edge_time + Timings::DATA_OUT_HOLD_TIME + 20, DATA_OUT, 0));
            break;
        case CpuState::T3:
            if (cycle_control == Constants8008::CycleControl::PCW)
            {
                next_events.push(
                        std::make_tuple(edge_time + Timings::DATA_OUT_HOLD_TIME + 20, DATA_OUT, 0));
            }
            break;
        case CpuState::T4:
            if (cycle_control == Constants8008::CycleControl::PCC)
            {
                next_events.push(std::make_tuple(edge_time + 20, DATA_OUT, 0));
            }
            break;
        case CpuState::T5:
            break;
    }
}

void CPU8008::signal_phase_1(Edge edge)
{
    if (input_pins.vdd == ::State::LOW)
    {
        set_next_activation_time(Scheduling::unscheduled());
        return;
    }

    auto edge_time = edge.time();

    if (is_rising(edge))
    {
        if (is_first_phase_cycle)
        {
            on_signal_11_raising(edge_time);
        }
        else
        {
            on_signal_12_raising(edge_time);
        }
    }
    else
    {
    }

    schedule_next_event(edge_time);
}

void CPU8008::signal_phase_2(Edge edge)
{
    if (input_pins.vdd == ::State::LOW)
    {
        set_next_activation_time(Scheduling::unscheduled());
        return;
    }

    auto edge_time = edge.time();

    if (is_rising(edge))
    {
        if (is_first_phase_cycle)
        {
            on_signal_21_raising(edge_time);
        }
    }
    else
    {
        if (is_first_phase_cycle)
        {
            on_signal_21_falling(edge_time);
        }
        else
        {
            on_signal_22_falling(edge_time);
        }
        is_first_phase_cycle = !is_first_phase_cycle;
    }

    schedule_next_event(edge_time);
}

void CPU8008::schedule_next_event(Scheduling::counter_type edge_time)
{
    if (next_events.empty())
    {
        set_next_activation_time(edge_time);
    }
    else
    {
        auto& next_event = next_events.top();
        set_next_activation_time(std::get<0>(next_event));
    }
    scheduler.change_schedule(get_id());
}

void CPU8008::signal_vdd(Edge edge)
{
    input_pins.vdd = edge.apply();
    if (input_pins.vdd == State::LOW)
    {
        set_next_activation_time(Scheduling::unscheduled());
    }
}
void CPU8008::signal_interrupt(Edge edge)
{
    input_pins.interrupt = edge.apply();
    if (input_pins.interrupt == ::State::HIGH)
    {
        // TODO: acknowledge the interruption with correct timing

        if ((edge.time() - input_pins.vdd.last_change()) < BOOT_UP_TIME)
        {
            // TODO: set garbage in the CPU state. It's too early
        }
        interrupt_pending = true;
    }
}

void CPU8008::signal_ready(Edge edge) { input_pins.ready = edge.apply(); }

void CPU8008::register_sync_trigger(std::function<void(Edge)> callback)
{
    output_pins.sync.subscribe(callback);
}

CPU8008::DebugData CPU8008::get_debug_data() const
{
    auto debug = CPU8008::DebugData{
            .instruction_register = instruction_register,
            .hidden_registers = hidden_registers,
            .latest_emitted_pci = latest_emitted_pci,
            .address_stack = address_stack.get_debug_data(),
            .decoded_instruction = decoded_instruction,
            .cycle_control = cycle_control,
    };

    std::memcpy(debug.registers, scratch_pad_memory, sizeof(uint8_t) * SCRATCH_PAD_SIZE);
    std::memcpy(debug.flags, flags, sizeof(uint8_t) * static_cast<size_t>(Flags::MAX));
    return debug;
}

void CPU8008::update_memory_cycle()
{
    if (cycle_control == CycleControl::PCI)
    {
        memory_cycle = 0;
    }
    else
    {
        memory_cycle = memory_cycle + 1;
    }
}

void CPU8008::execute_common_ti1_ti()
{
    assert(cycle_control != CycleControl::PCI);
    CycleActionsFor8008::T1_Action action =
            memory_cycle == 1 ? decoded_instruction.instruction->cycle_2.t1_action
                              : decoded_instruction.instruction->cycle_3.t1_action;

    switch (action)
    {
        case CycleActionsFor8008::Out_Reg_L:
            io_data_latch = scratch_pad_memory[static_cast<size_t>(Register::L)];
            break;
        case CycleActionsFor8008::Out_PC_L:
            io_data_latch = address_stack.get_low_pc_and_inc();
            break;
        case CycleActionsFor8008::Out_Reg_A:
            io_data_latch = scratch_pad_memory[static_cast<size_t>(Register::A)];
            break;
    }
}

void CPU8008::execute_t1i()
{
    update_memory_cycle();

    if (cycle_control == CycleControl::PCI)
    {
        instruction_register = 0;
        latest_emitted_pci = address_stack.get_pc();
        io_data_latch = address_stack.get_low_pc_no_inc();
    }
    else
    {
        execute_common_ti1_ti();
    }
}

void CPU8008::execute_t1()
{
    update_memory_cycle();

    if (cycle_control == CycleControl::PCI)
    {
        assert(memory_cycle == 0);
        instruction_register = 0;
        latest_emitted_pci = address_stack.get_pc();
        io_data_latch = address_stack.get_low_pc_and_inc();
    }
    else
    {
        execute_common_ti1_ti();
    }
}

void CPU8008::execute_t2()
{
    if (cycle_control == CycleControl::PCI)
    {
        assert(memory_cycle == 0);
        io_data_latch = address_stack.get_high_pc();
    }
    else
    {
        CycleActionsFor8008::T2_Action action =
                memory_cycle == 1 ? decoded_instruction.instruction->cycle_2.t2_action
                                  : decoded_instruction.instruction->cycle_3.t2_action;

        switch (action)
        {
            case CycleActionsFor8008::Out_Reg_H:
                io_data_latch = scratch_pad_memory[static_cast<size_t>(Register::H)];
                break;
            case CycleActionsFor8008::Out_PC_H:
                io_data_latch = address_stack.get_high_pc();
                break;
            case CycleActionsFor8008::Out_Reg_b_At_T2:
                io_data_latch = hidden_registers.b;
        }
    }
}

void CPU8008::execute_t3()
{
    if (cycle_control == CycleControl::PCI)
    {
        hidden_registers.a = 0x00; // only on RST? (but is it important?)
        hidden_registers.b = data_pins.read();

        assert(memory_cycle == 0);
        instruction_register = hidden_registers.b;
        decoded_instruction = instruction_table.decode_instruction(hidden_registers.b);

        CycleActionsFor8008::T3_Action action = decoded_instruction.instruction->cycle_1.t3_action;
        assert(action & CycleActionsFor8008::T3_Action::Fetch_IR_And_Reg_b);

        checks_cycle_end(static_cast<uint8_t>(action));
        checks_conditional_cycle_end(action);
    }
    else
    {
        assert(memory_cycle > 0);
        CycleActionsFor8008::T3_Action action =
                memory_cycle == 1 ? decoded_instruction.instruction->cycle_2.t3_action
                                  : decoded_instruction.instruction->cycle_3.t3_action;

        switch (action & CycleActionsFor8008::ACTION_MASK)
        {
            case CycleActionsFor8008::Fetch_Data_to_Reg_b:
                hidden_registers.b = data_pins.read();
                break;
            case CycleActionsFor8008::Fetch_Data_to_Reg_a:
                hidden_registers.a = data_pins.read();
                break;
            case CycleActionsFor8008::Out_Reg_b:
                assert(cycle_control == CycleControl::PCW);
                io_data_latch = hidden_registers.b;
                break;
            case CycleActionsFor8008::Fetch_IR_And_Reg_b:
            case CycleActionsFor8008::Halt:
                assert(false && "Invalid case");
        }

        // TODO: In the PCC case, the Idle action is supposed to acknowledge a WAIT signal to proceed.

        checks_cycle_end(static_cast<uint8_t>(action));
        checks_conditional_cycle_end(action);
    }
}

void CPU8008::checks_conditional_cycle_end(const CycleActionsFor8008::T3_Action& action)
{
    auto conditionally_ended = action & CycleActionsFor8008::CONDITIONAL_END;
    if (conditionally_ended)
    {
        auto condition = decoded_instruction.medium & 0b11;
        bool condition_verified = flags[condition];
        if (!(decoded_instruction.medium & 0b100))
        {
            condition_verified = !condition_verified;
        }

        if (!condition_verified)
        {
            ends_cycle(Constants8008::CycleControl::PCI);
        }
    }
}

void CPU8008::execute_t4()
{
    CycleActionsFor8008::T4_Action action =
            memory_cycle == 0
                    ? decoded_instruction.instruction->cycle_1.t4_action
                    : (memory_cycle == 1 ? decoded_instruction.instruction->cycle_2.t4_action
                                         : decoded_instruction.instruction->cycle_3.t4_action);

    switch (action & CycleActionsFor8008::ACTION_MASK)
    {
        case CycleActionsFor8008::Source_to_Reg_b: {
            auto source_register = decoded_instruction.low;
            assert(source_register != 0b111); // Memory is not a register
            hidden_registers.b = scratch_pad_memory[source_register];
        }
        break;
        case CycleActionsFor8008::Reg_a_to_PC_H:
            assert(cycle_control == CycleControl::PCR);
            address_stack.set_high_pc(hidden_registers.a);
            break;
        case CycleActionsFor8008::Push_And_Reg_a_to_PC_H:
            assert(cycle_control == CycleControl::PCR);
            address_stack.push();
            address_stack.set_high_pc(hidden_registers.a);
            break;
        case CycleActionsFor8008::Pop_Stack:
            address_stack.pop();
            break;
        case CycleActionsFor8008::Out_Conditions_Flags:
            assert(cycle_control == CycleControl::PCC);
            io_data_latch = flags[static_cast<size_t>(Flags::Sign)] |
                            (flags[static_cast<size_t>(Flags::Zero)] << 1) |
                            (flags[static_cast<size_t>(Flags::Parity)] << 2) |
                            (flags[static_cast<size_t>(Flags::Carry)] << 3);
    }

    checks_cycle_end(static_cast<uint8_t>(action));
}

void CPU8008::checks_cycle_end(uint8_t action)
{
    cycle_ended = action & CycleActionsFor8008::CYCLE_END;

    if (cycle_ended)
    {
        if (memory_cycle == 0)
        {
            auto& next_cycle = decoded_instruction.instruction->cycle_2;
            ends_cycle(next_cycle.cycle_control);
        }
        else
        {
            assert(memory_cycle == 1);
            auto& next_cycle = decoded_instruction.instruction->cycle_3;
            ends_cycle(next_cycle.cycle_control);
        }
    }
}

void CPU8008::execute_t5()
{
    CycleActionsFor8008::T5_Action action =
            memory_cycle == 0
                    ? decoded_instruction.instruction->cycle_1.t5_action
                    : (memory_cycle == 1 ? decoded_instruction.instruction->cycle_2.t5_action
                                         : decoded_instruction.instruction->cycle_3.t5_action);

    switch (action & CycleActionsFor8008::ACTION_MASK)
    {
        case CycleActionsFor8008::Reg_b_to_Destination: {
            auto destination_register = decoded_instruction.medium;
            assert(destination_register != 0b111); // Memory is not a register
            scratch_pad_memory[destination_register] = hidden_registers.b;
        }
        break;
        case CycleActionsFor8008::Inc_Destination: {
            auto destination_register = decoded_instruction.medium;
            assert(destination_register != 000); // This is HLT, cannot INA
            assert(destination_register != 111); // Cannot increase Memory with INr

            auto& reg = scratch_pad_memory[destination_register];
            reg += 1;

            update_flags(reg);
            // Carry is not updated by INr
        }
        break;
        case CycleActionsFor8008::Dec_Destination: {
            auto destination_register = decoded_instruction.medium;
            assert(destination_register != 000); // This is HLT, cannot DCA
            assert(destination_register != 111); // Cannot decrease Memory with DCr

            auto& reg = scratch_pad_memory[destination_register];
            reg -= 1;

            update_flags(reg);
            // Carry is not updated by DCr
        }
        break;
        case CycleActionsFor8008::ALU_Operation_With_RegB: {
            auto operation = decoded_instruction.medium;
            auto& register_A = scratch_pad_memory[static_cast<size_t>(Register::A)];

            switch (operation)
            {
                case 0b000: // ADD
                {
                    uint16_t intermediate = static_cast<uint16_t>(register_A) +
                                            static_cast<uint16_t>(hidden_registers.b);
                    flags[static_cast<size_t>(Flags::Carry)] = intermediate > 255;
                    scratch_pad_memory[static_cast<size_t>(Register::A)] =
                            static_cast<uint8_t>(intermediate);
                    update_flags(register_A);
                }
                break;
                case 0b001: // ADD with Carry
                {
                    uint16_t intermediate = static_cast<uint16_t>(register_A) +
                                            static_cast<uint16_t>(hidden_registers.b) +
                                            flags[static_cast<size_t>(Flags::Carry)];
                    flags[static_cast<size_t>(Flags::Carry)] = intermediate > 255;
                    scratch_pad_memory[static_cast<size_t>(Register::A)] =
                            static_cast<uint8_t>(intermediate);
                    update_flags(register_A);
                }
                break;
                case 0b010: // SUB
                {
                    int16_t intermediate = static_cast<int16_t>(register_A) -
                                           static_cast<int16_t>(hidden_registers.b);
                    flags[static_cast<size_t>(Flags::Carry)] = intermediate < 255;
                    scratch_pad_memory[static_cast<size_t>(Register::A)] =
                            static_cast<uint8_t>(intermediate);
                    update_flags(register_A);
                }
                break;
                case 0b011: // SUB with Carry
                {
                    int16_t intermediate = static_cast<int16_t>(register_A) -
                                           static_cast<int16_t>(hidden_registers.b) -
                                           flags[static_cast<size_t>(Flags::Carry)];
                    flags[static_cast<size_t>(Flags::Carry)] = intermediate < 255;
                    scratch_pad_memory[static_cast<size_t>(Register::A)] =
                            static_cast<uint8_t>(intermediate);
                    update_flags(register_A);
                }

                break;
                case 0b100: // AND
                    scratch_pad_memory[static_cast<size_t>(Register::A)] &=
                            static_cast<uint8_t>(hidden_registers.b);
                    update_flags(register_A);
                    flags[static_cast<size_t>(Flags::Carry)] = 0;
                    break;
                case 0b101: // XOR
                    scratch_pad_memory[static_cast<size_t>(Register::A)] ^=
                            static_cast<uint8_t>(hidden_registers.b);
                    update_flags(register_A);
                    flags[static_cast<size_t>(Flags::Carry)] = 0;
                    break;
                case 0b110: // OR
                    scratch_pad_memory[static_cast<size_t>(Register::A)] |=
                            static_cast<uint8_t>(hidden_registers.b);
                    update_flags(register_A);
                    flags[static_cast<size_t>(Flags::Carry)] = 0;
                    break;
                case 0b111: // CP (Compare)
                {
                    int16_t intermediate = static_cast<int16_t>(register_A) -
                                           static_cast<int16_t>(hidden_registers.b);
                    flags[static_cast<size_t>(Flags::Carry)] = intermediate < 255;
                    flags[static_cast<size_t>(Flags::Zero)] = intermediate == 0;
                    // TODO: What about Sign and Parity?
                }
                break;
                default:
                    assert(false && "This operation is not supposed to exist.");
            }
        }
        break;
        case CycleActionsFor8008::Rotate_A: {
            auto rotate_op = decoded_instruction.medium & 0b11;
            auto& register_A = scratch_pad_memory[static_cast<size_t>(Register::A)];

            switch (rotate_op)
            {
                case 0b00: // RLC
                    flags[static_cast<size_t>(Flags::Carry)] = (register_A & 0b10000000) >> 7;
                    register_A = (register_A << 1) & 0b11111110;
                    register_A |= flags[static_cast<size_t>(Flags::Carry)];
                    break;
                case 0b01: // RRC
                    flags[static_cast<size_t>(Flags::Carry)] = (register_A & 0b00000001);
                    register_A = (register_A >> 1) & 0b01111111;
                    register_A |= flags[static_cast<size_t>(Flags::Carry)] << 7;
                    break;
                case 0b10: // RAL
                {
                    auto previous_carry = flags[static_cast<size_t>(Flags::Carry)];
                    flags[static_cast<size_t>(Flags::Carry)] = (register_A & 0b10000000) >> 7;
                    register_A = (register_A << 1) & 0b11111110;
                    register_A |= previous_carry;
                }
                break;
                case 0b11: // RAR
                {
                    auto previous_carry = flags[static_cast<size_t>(Flags::Carry)];
                    flags[static_cast<size_t>(Flags::Carry)] = (register_A & 0b00000001);
                    register_A = (register_A >> 1) & 0b01111111;
                    register_A |= previous_carry << 7;
                }
                break;
                default:
                    assert(false && "This operation is not supposed to exist.");
            }
        }
        break;
        case CycleActionsFor8008::Reg_b_to_PC_L:
            address_stack.set_low_pc(hidden_registers.b);
            break;
        case CycleActionsFor8008::Reg_b_to_PC_L_3_to_5: {
            uint8_t rst_address = decoded_instruction.medium << 3;
            address_stack.set_low_pc(rst_address);
        }
        break;
        case CycleActionsFor8008::Reg_b_to_A: {
            auto& register_A = scratch_pad_memory[static_cast<size_t>(Register::A)];
            register_A = hidden_registers.b;
        }
        break;
    }

    cycle_ended = true;
}

void CPU8008::update_flags(const uint8_t& reg)
{
    // Updates each flags except Carry
    flags[static_cast<size_t>(Flags::Zero)] = (reg == 0);
    flags[static_cast<size_t>(Flags::Sign)] = (reg & 0x80);
    flags[static_cast<size_t>(Flags::Parity)] = ((reg & 0x1) == 0);
}

void CPU8008::ends_cycle(Constants8008::CycleControl new_cycle_control)
{
    cycle_ended = true;
    next_cycle_control = new_cycle_control;
}

bool CPU8008::is_instruction_complete() const
{
    assert(*output_pins.state == Constants8008::CpuState::T3 ||
           *output_pins.state == Constants8008::CpuState::T4);
    return next_cycle_control == Constants8008::CycleControl::PCI;
}

void CPU8008::interrupt(Scheduling::counter_type edge_time)
{
    interrupt_pending = false;

    next_events.push(std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T1I)));
}

void CPU8008::register_state_change(state_callback_type callback)
{
    output_pins.state.subscribe(callback);
}
