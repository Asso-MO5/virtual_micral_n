#include "CPU8008.h"
#include <emulation_core/src/Edge.h>

namespace
{
    const Scheduling::counter_type BOOT_UP_TIME = 16'000;

    namespace Timings
    {
        const Scheduling::counter_type MIN_CLOCK_PERIOD = 2000;
        const Scheduling::counter_type MAX_CLOCK_PERIOD = 3000;
        const Scheduling::counter_type PULSE_WIDTH_PHASE_1 = 700;
        const Scheduling::counter_type PULSE_WIDTH_PHASE_2 = 550;
        const Scheduling::counter_type FALLING_1_TO_FALLING_2_EDGE_MIN = 900;
        const Scheduling::counter_type FALLING_1_TO_FALLING_2_EDGE_MAX = 1100;
        const Scheduling::counter_type DELAY_FROM_PHASE_1_TO_2 = 200;
        const Scheduling::counter_type DELAY_FROM_PHASE_2_TO_1 = 400;
    } // namespace Timings

} // namespace

CPU8008::CPU8008()
{
    output_pins.state = CpuState::STOPPED;
    output_pins.sync = ::State::LOW;
}

void CPU8008::step()
{
    set_next_activation_time(get_next_activation_time() +
                             Timings::MIN_CLOCK_PERIOD); // Doing nothing at the monent.
}

const CPU8008::OutputPins& CPU8008::get_output_pins() const { return output_pins; }
const CPU8008::DataPins& CPU8008::get_data_pins() const { return data_pins; }

void CPU8008::signal_phase_1(Edge edge)
{
    if (input_pins.vdd == ::State::LOW)
    {
        set_next_activation_time(Scheduling::unscheduled());
        return;
    }

    auto edge_time = edge.time();
    set_next_activation_time(edge_time);

    if (edge == Edge::Front::RISING)
    {
        switch (output_pins.state)
        {
            case CpuState::STOPPED:
                // Halted and boot up state.

                if (input_pins.interrupt == ::State::HIGH)
                {
                    // TODO: acknowledge the interruption with correct timing
                    // (by timestamping the state change for example)

                    if ((edge_time - input_pins.vdd.last_change()) < BOOT_UP_TIME)
                    {
                        // TODO: set garbage in the CPU state. It's too early
                    }

                    output_pins.state = CpuState::T1I;
                }
                break;
            case CpuState::T1:
                // Memory address emission step 1
                // TODO: advance PC
                [[fallthrough]];
            case CpuState::T1I:
                // TODO: set PC low address on BUS (with good timing)
                // The step() function will process the instruction and switch to T2 state
                break;
            case CpuState::T2:
                // Memory address emission step 2
                // TODO: set PC high address on BUS (with good timing)
                // The step() function will process the instruction and switch to WAIT or T3 state
                // depending on the READY signal.
                break;
            case CpuState::T3:
                // Data Fetch
                // TODO: schedule the DATA fetch and decode the instruction.
                // The step() function will process the instruction and switch to
                // STOPPED if HALT
                // T4 is instruction asks
                // STOPPED if interrupted at instruction cycle end
                // T1 if finished
                // T1I if long instruction jammed
                break;
            case CpuState::WAIT:
                // Waiting state
                // TODO: wait until READY is not asserted anymore.
                break;
            case CpuState::T4:
                // Instruction execution step 1
                // The step() function will process the instruction and switch to
                // T5 is instruction asks
                // STOPPED if interrupted at instruction cycle end
                // T1 if finished
                // T1I if long instruction jammed
                break;
            case CpuState::T5:
                // Instruction execution step 2
                // The step() function will process the instruction and switch to
                // STOPPED if interrupted at instruction cycle end
                // T1 if finished
                break;
        }
    }
}

void CPU8008::signal_phase_2(Edge edge) {}

void CPU8008::signal_vdd(Edge edge) { input_pins.vdd = edge.apply(); }
void CPU8008::signal_interrupt(Edge edge) { input_pins.interrupt = edge.apply(); }

AddressStack::AddressStack() { clear_stack(); }
AddressStack::AddressStack(uint16_t pc)
{
    clear_stack();
    stack[stack_index] = pc;
}

void AddressStack::clear_stack()
{
    std::fill(begin(stack), end(stack), 0);
    stack_index = 0;
}

uint16_t AddressStack::get_pc() const { return stack[stack_index]; }

uint16_t AddressStack::get_low_pc_and_inc()
{
    // Note: the 8008 normally waits T2 to increment the 6 high bits part of the PC
    // It's done directly in the simulation, as T1 and T2 are not supposed to be interrupted.
    auto& pc = stack[stack_index];
    emitted_pc = pc;
    pc = (pc + 1) & 0x3fff;
    return emitted_pc & 0xff;
}

uint16_t AddressStack::get_high_pc_and_inc() const { return (emitted_pc & 0x3f00) >> 8; }

void AddressStack::push(uint16_t address)
{
    stack_index = (stack_index + 1) % stack.size();
    stack[stack_index] = address;
}
void AddressStack::pop()
{
    stack_index = (stack_index - 1);
    if (stack_index < 0)
    {
        stack_index = stack.size() - 1;
    }
}
