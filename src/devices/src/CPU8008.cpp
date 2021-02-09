#include "CPU8008.h"
#include <emulation_core/src/Edge.h>

#include <iostream>
#include <utility>

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
            next_events.push(
                    std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T1)));

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
    return {.pc = address_stack.get_pc(),
            .instruction_register = instruction_register,
            .hidden_registers = hidden_registers};
}

namespace Instruction
{
    bool is_LrI(uint8_t instruction_register)
    {
        return (instruction_register & 0b11000000) == 0b00000000 &&
               (instruction_register & 0b00000111) == 0b110;
    }

    bool is_Lrr(uint8_t instruction_register)
    {
        return (instruction_register & 0b11000000) == 0b11000000;
    }

} // namespace Instruction

void CPU8008::execute_t1i()
{
    switch (cycle_control)
    {
        case CycleControl::PCI:
            io_data_latch = address_stack.get_low_pc_no_inc();
            break;
        case CycleControl::PCR:
            if (Instruction::is_LrI(instruction_register))
            {
                io_data_latch = scratch_pad_memory[static_cast<size_t>(Register::L)];
            }

            break;
        case CycleControl::PCC:
            break;
        case CycleControl::PCW:
            break;
    }
}

void CPU8008::execute_t1()
{
    switch (cycle_control)
    {
        case CycleControl::PCI:
            io_data_latch = address_stack.get_low_pc_and_inc();
            break;
        case CycleControl::PCR: {
            if (Instruction::is_LrI(instruction_register))
            {
                io_data_latch = address_stack.get_low_pc_and_inc();
            }
        }
        break;
        case CycleControl::PCC:
            break;
        case CycleControl::PCW:
            break;
    }
}

void CPU8008::execute_t2()
{
    switch (cycle_control)
    {
        case CycleControl::PCI:
            io_data_latch = address_stack.get_high_pc();
            break;
        case CycleControl::PCR: {
            if (Instruction::is_LrI(instruction_register))
            {
                io_data_latch = address_stack.get_high_pc();
            }
        }
        break;
        case CycleControl::PCC:
            break;
        case CycleControl::PCW:
            break;
    }
}

void CPU8008::execute_t3()
{
    if (cycle_control == CycleControl::PCI | cycle_control == CycleControl::PCR)
    {
        hidden_registers.a = 0x00; // only on RST? (but is it important?)
        hidden_registers.b = data_pins.read();
    }

    switch (cycle_control)
    {
        case CycleControl::PCI: {
            instruction_register = hidden_registers.b;

            if (Instruction::is_LrI(instruction_register))
            {
                cycle_ended = true;
                cycle_control = CycleControl::PCR;
            }
        }
        break;
        case CycleControl::PCR:
            // LrI : Nothing to do. DATA is in reg.b
            break;
        case CycleControl::PCC:
            break;
        case CycleControl::PCW:
            break;
    }
}

void CPU8008::execute_t4()
{
    switch (cycle_control)
    {
        case CycleControl::PCI: {
            if (Instruction::is_Lrr(instruction_register))
            {
                auto source_register = instruction_register & 0b111;
                if (source_register == 0b111)
                {
                    // TODO: LrM
                }
                else
                {
                    hidden_registers.b = source_register;
                }
            }
        }
        break;
        case CycleControl::PCR:
            // LrI -> nothing to do. Cycle skipped.
            break;
        case CycleControl::PCC:
            break;
        case CycleControl::PCW:
            break;
    }
}

void CPU8008::execute_t5()
{
    switch (cycle_control)
    {
        case CycleControl::PCI: {
            if (cycle_control == CycleControl::PCI)
            {
                if (Instruction::is_Lrr(instruction_register))
                {
                    assert(hidden_registers.b != 0b111);
                    auto destination_register = (instruction_register & 0b111000) >> 3;
                    scratch_pad_memory[destination_register] = hidden_registers.b;
                }
            }
        }
        break;
        case CycleControl::PCR: {
            if (Instruction::is_LrI(instruction_register))
            {
                auto destination_register = (instruction_register & 0b111000) >> 3;
                assert(destination_register != 0b111);

                scratch_pad_memory[destination_register] = hidden_registers.b;
            }
        }
        break;
        case CycleControl::PCC:
            break;
        case CycleControl::PCW:
            break;
    }
}
