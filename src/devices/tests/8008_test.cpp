#include "CPU8008.h"

#include <emulation_core/src/Frequency.h>

#include "gmock/gmock.h"

using namespace testing;

TEST(CPU8008, starts_in_stop_state)
{
    CPU8008 cpu;

    auto outputs = cpu.get_output_pins();
    ASSERT_THAT(outputs.state, Eq(CPU8008::CpuState::STOPPED));
}

TEST(CPU8008, switches_to_t1i_after_start_when_clocked)
{
    CPU8008 cpu;

    Frequency freq{500'000_hz};

    // The CPU needs 16 Clock Cycles to initialize, in STOPPED state.
    // After that, the INTERRUPT must go to Logic 1 to exit the STOPPED state and go to TI1.
    int START_UP_CYCLES = 16;
    Scheduling::counter_type time_counter = 0; // Faking a clock for Phase 1.

    cpu.signal_vdd(Edge::Front::RISING); // Power the CPU
    cpu.signal_interrupt(Edge::Front::FALLING); // Interrupt in Logic 0

    for (int clock_cycles = 0; clock_cycles < START_UP_CYCLES; ++clock_cycles)
    {
        cpu.signal_phase_1(Edge{Edge::Front::RISING, time_counter});
        cpu.step();

        ASSERT_THAT(cpu.get_output_pins().state, Eq(CPU8008::CpuState::STOPPED));

        cpu.signal_phase_1(Edge{Edge::Front::FALLING, time_counter + 1});
        time_counter += freq.get_period_as_ns();
    }

    cpu.signal_interrupt(Edge{Edge::Front::RISING, time_counter});
    cpu.signal_phase_1(Edge{Edge::Front::RISING, time_counter});
    cpu.step();
    time_counter += freq.get_period_as_ns();
    cpu.signal_phase_1(Edge{Edge::Front::FALLING, time_counter});
    cpu.signal_interrupt(Edge{Edge::Front::FALLING, time_counter + 201});
    cpu.step();

    ASSERT_THAT(cpu.get_output_pins().state, Eq(CPU8008::CpuState::T1I));
    // ASSERT_THAT(cpu.get_data_pins().data, Eq(0x0000)); <- No, it has to wait phase_21
}

TEST(AddressStack, initializes_with_pc_at_0)
{
    AddressStack address_stack{};

    ASSERT_THAT(address_stack.get_pc(), Eq(0x0000));
}

TEST(AddressStack, get_provide_low_and_high_pc_address_and_increment_it)
{
    AddressStack address_stack{0x10fe};

    ASSERT_THAT(address_stack.get_low_pc_and_inc(), Eq(0xfe));
    ASSERT_THAT(address_stack.get_high_pc_and_inc(), Eq(0x10));
    ASSERT_THAT(address_stack.get_low_pc_and_inc(), Eq(0xff));
    ASSERT_THAT(address_stack.get_high_pc_and_inc(), Eq(0x10));
    ASSERT_THAT(address_stack.get_low_pc_and_inc(), Eq(0x00));
    ASSERT_THAT(address_stack.get_high_pc_and_inc(), Eq(0x11));
}

TEST(AddressStack, pc_is_14_bit_wide)
{
    AddressStack address_stack{0x3fff};

    ASSERT_THAT(address_stack.get_low_pc_and_inc(), Eq(0xff));
    ASSERT_THAT(address_stack.get_high_pc_and_inc(), Eq(0x3f));
    ASSERT_THAT(address_stack.get_low_pc_and_inc(), Eq(0x00));
    ASSERT_THAT(address_stack.get_high_pc_and_inc(), Eq(0x00));

    AddressStack address_stack_2{0xbfff};
    ASSERT_THAT(address_stack_2.get_low_pc_and_inc(), Eq(0xff));
    ASSERT_THAT(address_stack_2.get_high_pc_and_inc(), Eq(0x3f));

}

TEST(AddressStack, can_push_an_address)
{
    AddressStack address_stack{0x1000};

    address_stack.push(0x2000);

    ASSERT_THAT(address_stack.get_low_pc_and_inc(), Eq(0x00));
    ASSERT_THAT(address_stack.get_high_pc_and_inc(), Eq(0x20));
}

TEST(AddressStack, can_pop_an_address)
{
    AddressStack address_stack{0x1000};

    address_stack.push(0x2000);
    address_stack.pop();

    ASSERT_THAT(address_stack.get_low_pc_and_inc(), Eq(0x00));
    ASSERT_THAT(address_stack.get_high_pc_and_inc(), Eq(0x10));
}



/*
 * Next Tests:
 *
 * - At startup time, the CPU needs 16 clock periods to clear its internal memory (can be simulated by not reacting during this time)
 * - If nothing is done, the PC is 0 in T1I
 * - Test startup with NOP at address 0 (how to fetch memory ?)
 * - Test startup with RST at address 0
 * - Test startup with Jamming with a RST (a RET should go back to PC = 0)
 */