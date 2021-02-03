#include "CPU8008.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(CPU8008, starts_in_stop_state)
{
    CPU8008 cpu;

    auto outputs = cpu.get_output_pins();
    ASSERT_THAT(outputs.state, Eq(CPU8008::CpuState::STOPPED));
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