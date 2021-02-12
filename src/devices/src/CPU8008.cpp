#include "CPU8008.h"
#include <emulation_core/src/Edge.h>

#include <cstring>
#include <iostream>
#include <utility>

using namespace Constants8008;

namespace
{
    const Scheduling::counter_type BOOT_UP_TIME = 16'000;

    namespace Timings
    {
        using namespace Scheduling;
        const counter_type MIN_CLOCK_PERIOD = 2000;
        const counter_type MAX_CLOCK_PERIOD = 3000;
        const counter_type PULSE_WIDTH_PHASE_1 = 700;
        const counter_type PULSE_WIDTH_PHASE_2 = 550;
        const counter_type FALLING_1_TO_FALLING_2_EDGE_MIN = 900;
        const counter_type FALLING_1_TO_FALLING_2_EDGE_MAX = 1100;
        const counter_type DELAY_FROM_PHASE_1_TO_2 = 200;
        const counter_type DELAY_FROM_PHASE_2_TO_1 = 400;
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
    output_pins.sync = ::State::LOW;
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
            output_pins.sync = sync_edge.apply();
            sync_callback(sync_edge);
            // TODO : Constraint :  tSD max (.70) after ø21 FALL
            break;
        }
        case STATE: {
            // TODO: Constraint : tS2 max (1.0) after ø11 RAISE (for T1/TI1)
            // TODO: Constraint : tS1 max (1.1) after ø11 RAISE (for others)
            output_pins.state = static_cast<CpuState>(param);

            switch (output_pins.state)
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
                    // Too early to execute T3.
                    // It is executed on the DATA_IN signal.
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
                data_pins.take_bus();
                data_pins.write(io_data_latch);
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

    switch (output_pins.state)
    {
        case CpuState::STOPPED:
            if (interrupt_pending)
            {
                interrupt_pending = false;

                next_events.push(
                        std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T1I)));
            }
            break;
        case CpuState::WAIT:
            break;
        case CpuState::T1:
        case CpuState::T1I:
            next_events.push(
                    std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T2)));
            break;
        case CpuState::T2:
            next_events.push(
                    std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T3)));
            break;
        case CpuState::T3:
            if (cycle_ended)
            {
                // TODO: Test Interruption
                // TODO: Instruction Jammed goes to TI1
                next_events.push(
                        std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T1)));
            }
            else
            {
                next_events.push(
                        std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T4)));
            }
            break;
        case CpuState::T4:
            next_events.push(
                    std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T5)));
            break;
        case CpuState::T5:
            assert(cycle_ended);
            // TODO: Test Interruption
            next_events.push(
                    std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T1)));
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
    switch (output_pins.state)
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
        case CpuState::T4:
        case CpuState::T5:
            break;
    }
}

void CPU8008::on_signal_21_falling(Scheduling::counter_type edge_time)
{
    if (output_pins.state == CpuState::T3)
    {
        next_events.push(std::make_tuple(edge_time + Timings::DATA_IN_HOLD_TIME, DATA_IN, 1));
    }
}

void CPU8008::on_signal_22_falling(Scheduling::counter_type edge_time)
{
    switch (output_pins.state)
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
        case CpuState::T4:
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

    if (edge == Edge::Front::RISING)
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

    if (edge == Edge::Front::RISING)
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
void CPU8008::register_sync_trigger(std::function<void(Edge)> callback)
{
    sync_callback = std::move(callback);
}

CPU8008::DebugData CPU8008::get_debug_data() const
{
    auto debug = CPU8008::DebugData{
            .pc = address_stack.get_pc(),
            .instruction_register = instruction_register,
            .hidden_registers = hidden_registers,
    };

    std::memcpy(debug.registers, scratch_pad_memory, sizeof(uint8_t) * SCRATCH_PAD_SIZE);
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
            assert(false && "Not done yet");
    }
}

void CPU8008::execute_t1i()
{
    update_memory_cycle();

    if (cycle_control == CycleControl::PCI)
    {
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
                assert(false && "Not done yet");
        }
    }
}

void CPU8008::execute_t3()
{
    if (cycle_control == CycleControl::PCI)
    {
        hidden_registers.a = 0x00; // only on RST? (but is it important?)
        hidden_registers.b = data_pins.read();
    }
    else if (cycle_control == CycleControl::PCR)
    {
        auto& cycle = (memory_cycle == 1) ? decoded_instruction.instruction->cycle_2
                                          : decoded_instruction.instruction->cycle_3;

        switch (cycle.t3_action & 15)
        {
            case CycleActionsFor8008::T3_Action::Fetch_Data_to_Reg_b:
                hidden_registers.b = data_pins.read();
                break;
            case CycleActionsFor8008::T3_Action::Fetch_Data_to_Reg_a:
                hidden_registers.a = data_pins.read();
                break;
            default:
                assert(false && "Invalid situation");
        }
    }

    if (cycle_control == CycleControl::PCI)
    {
        assert(memory_cycle == 0);
        instruction_register = hidden_registers.b;
        decoded_instruction = instruction_table.decode_instruction(hidden_registers.b);

        CycleActionsFor8008::T3_Action action = decoded_instruction.instruction->cycle_1.t3_action;
        assert(action & CycleActionsFor8008::T3_Action::Fetch_IR_And_Reg_b);

        checks_cycle_end(static_cast<uint8_t>(action));

        auto conditionally_ended = action & CycleActionsFor8008::CONDITIONAL_END;
        if (conditionally_ended)
        {
            auto condition = (instruction_register & 0b111000) >> 3;
            bool condition_verified = flags[condition & 0b11];
            if (!(condition_verified & 0b100))
            {
                condition_verified = !condition_verified;
            }

            if (!condition_verified)
            {
                cycle_ended = true;
                // And stays in PCI
            }
        }
    }
    else
    {
        assert(memory_cycle > 0);
        CycleActionsFor8008::T3_Action action =
                memory_cycle == 1 ? decoded_instruction.instruction->cycle_2.t3_action
                                  : decoded_instruction.instruction->cycle_3.t3_action;

        switch (action)
        {
            case CycleActionsFor8008::Fetch_Data_to_Reg_b:
                hidden_registers.b = data_pins.read();
                break;
            case CycleActionsFor8008::Fetch_Data_to_Reg_a:
                hidden_registers.a = data_pins.read();
                break;
            case CycleActionsFor8008::Out_Reg_b:
                assert(false && "Not done yet");
                break;
            case CycleActionsFor8008::Fetch_IR_And_Reg_b:
            case CycleActionsFor8008::Halt:
                assert(false && "Invalid case");
        }

        checks_cycle_end(static_cast<uint8_t>(action));
    }
}

void CPU8008::execute_t4()
{
    CycleActionsFor8008::T4_Action action =
            memory_cycle == 0
                    ? decoded_instruction.instruction->cycle_1.t4_action
                    : (memory_cycle == 1 ? decoded_instruction.instruction->cycle_2.t4_action
                                         : decoded_instruction.instruction->cycle_3.t4_action);

    switch (action)
    {
        case CycleActionsFor8008::Source_to_Reg_b: {
            auto source_register = instruction_register & 0b111;
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
            assert(false && "Not done yet");
    }

    checks_cycle_end(static_cast<uint8_t>(action));
}

void CPU8008::execute_t5()
{
    CycleActionsFor8008::T5_Action action =
            memory_cycle == 0
                    ? decoded_instruction.instruction->cycle_1.t5_action
                    : (memory_cycle == 1 ? decoded_instruction.instruction->cycle_2.t5_action
                                         : decoded_instruction.instruction->cycle_3.t5_action);

    switch (action)
    {
        case CycleActionsFor8008::Reg_b_to_Destination: {
            assert(hidden_registers.b != 0b111); // Memory is not a register
            auto destination_register = (instruction_register & 0b111000) >> 3;
            scratch_pad_memory[destination_register] = hidden_registers.b;
        }
        break;
        case CycleActionsFor8008::Inc_Destination: {
            auto destination_register = (instruction_register & 0b111000) >> 3;
            assert(destination_register != 000); // This is HLT, cannot INA
            assert(destination_register != 111); // Cannot increase Memory with INr

            auto& reg = scratch_pad_memory[destination_register];
            reg += 1;

            flags[static_cast<size_t>(Flags::Zero)] = (reg == 0);
            flags[static_cast<size_t>(Flags::Sign)] = (reg & 0x80);
            flags[static_cast<size_t>(Flags::Parity)] = ((reg & 0x1) == 0);
            // Carry is not updated by INr
        }
        break;
        case CycleActionsFor8008::Dec_Destination:
            assert(false && "Not done yet");
            break;
        case CycleActionsFor8008::ALU_Operation_With_RegB:
            assert(false && "Not done yet");
            break;
        case CycleActionsFor8008::Rotate_A:
            assert(false && "Not done yet");
            break;
        case CycleActionsFor8008::Reg_b_to_PC_L:
            address_stack.set_low_pc(hidden_registers.b);
            break;
        case CycleActionsFor8008::Reg_b_to_PC_L_3_to_5:
            assert(false && "Not done yet");
            break;
        case CycleActionsFor8008::Reg_b_to_A:
            assert(false && "Not done yet");
            break;
    }

    cycle_ended = true;
}

void CPU8008::checks_cycle_end(uint8_t action)
{
    cycle_ended = action & CycleActionsFor8008::CYCLE_END;

    if (cycle_ended)
    {
        if (memory_cycle == 0)
        {
            auto& next_cycle = decoded_instruction.instruction->cycle_2;
            cycle_control = next_cycle.cycle_control;
        }
        else
        {
            assert(memory_cycle == 1);
            auto& next_cycle = decoded_instruction.instruction->cycle_3;
            cycle_control = next_cycle.cycle_control;
        }
    }
}
