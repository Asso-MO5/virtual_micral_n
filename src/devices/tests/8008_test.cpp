#include "CPU8008.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(CPU8008, starts_in_stop_state)
{
    CPU8008 cpu;

    auto outputs = cpu.get_output_pins();
    ASSERT_THAT(outputs.state, Eq(CPU8008::State::STOPPED));
}

TEST(CPU8008, switches_to_t1i_after_start)
{
    CPU8008 cpu;

    int step_timeout = 10;
    while (step_timeout > 0)
    {
        cpu.step();
        if (cpu.get_output_pins().state == CPU8008::State::T1I)
        {
            break;
        }
        step_timeout -= 1;
    }
    ASSERT_NE(step_timeout, 0); // Timeout
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