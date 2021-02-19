#ifndef MICRALN_CONSTANTS8008_H
#define MICRALN_CONSTANTS8008_H

#include <cstdint>

namespace Constants8008
{
    // All values are shifted to high bits so they can be easily OR'd with address on T2.
    enum class CycleControl : uint8_t
    {
        PCI = 0b00000000, // Memory read for the first byte of instruction
        PCR = 0b01000000, // Memory read for data or additional bytes of instruction
        PCC = 0b10000000, // Data is a command for I/O operation
        PCW = 0b11000000, // Memory write for data
    };

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

} // namespace Constants8008

static const char* STATE_NAMES[] = {"WAIT", "T3", "T1", "STOPPED", "T2", "T5", "T1I", "T4"};
static const char* REGISTER_NAMES[] = {"A", "B", "C", "D", "E", "H", "L"};
static const char* FLAG_NAMES[] = {"Carry ", "Zero  ", "Sign  ", "Parity"};
static const char* CYCLE_CONTROL_NAMES[] = {"PCI ", "PCR", "PCC", "PCW"};

#endif //MICRALN_CONSTANTS8008_H
