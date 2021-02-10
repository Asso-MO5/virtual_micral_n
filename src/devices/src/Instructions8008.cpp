#include "Instructions8008.h"

InstructionTableFor8008::DecodedInstruction
InstructionTableFor8008::decode_instruction(uint8_t opcode)
{
    static Instruction unknown_instruction{InstructionNameFor8008::UNKNOWN};

    uint8_t high_part = (opcode & 0b11000000) >> 6;
    uint8_t medium_part = (opcode & 0b00111000) >> 3;
    uint8_t low_part = (opcode & 0b00000111) >> 0;

    for (auto& [candidate, decode_high, decode_medium, decode_low] : decoding_table)
    {
        if (decode_high != high_part)
        {
            continue;
        }
        auto mask_low = (decode_low | 0b111000) >> 3;
        auto result_low = low_part & mask_low;
        if (result_low != (decode_low & 0b111))
        {
            continue;
        }
        auto mask_medium = (decode_medium | 0b111000) >> 3;
        auto result_medium = medium_part & mask_medium;
        if (result_medium != (decode_medium & 0b111))
        {
            continue;
        }

        auto& found = instruction_table[candidate];
        assert(found.name == candidate); // Both table must be synchronized

        return {found, medium_part, low_part};
    }
    return {unknown_instruction, 0, 0};
}

InstructionTableFor8008::InstructionTableFor8008()
{
    using namespace InstructionNameFor8008;
    using namespace CycleActionsFor8008;

    instruction_table = {
            // Index Register Instructions
            {Lrr, FirstCycle{T3(Fetch_IR_And_RegB), T4(Source_to_RegB), T5(RegB_to_Destination)},
             OtherCycle{}, OtherCycle{}, "Lrr"},
            {LrM, FirstCycle{T3(Fetch_IR_And_RegB | CYCLE_END)},
             OtherCycle{CPU8008::CycleControl::PCR, T1(Out_Reg_L), T2(Out_Reg_H),
                        T3(Fetch_Data_to_RegB), T4(IDLE), T5(RegB_to_Destination)},
             OtherCycle{}, "LrM"},
            {LMr, FirstCycle{T3(Fetch_IR_And_RegB), T4(Source_to_RegB | CYCLE_END)},
             OtherCycle{CPU8008::CycleControl::PCW, T1(Out_Reg_L), T2(Out_Reg_H),
                        T3(Out_RegB | CYCLE_END)},
             OtherCycle{}, "LMr"},
            {LrI, FirstCycle{T3(Fetch_IR_And_RegB | CYCLE_END)},
             OtherCycle{CPU8008::CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),
                        T3(Fetch_Data_to_RegB), T4(IDLE), T5(RegB_to_Destination)},
             OtherCycle{}, "LrI"},
            {LMI, FirstCycle{T3(Fetch_IR_And_RegB | CYCLE_END)},
             OtherCycle{CPU8008::CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),
                        T3(Fetch_Data_to_RegB | CYCLE_END)},
             OtherCycle{CPU8008::CycleControl::PCW, T1(Out_Reg_L), T2(Out_Reg_H),
                        T3(Out_RegB | CYCLE_END)},
             "LMI"},
            {INr, FirstCycle{T3(Fetch_IR_And_RegB), T4(IDLE), T5(Inc_Destination)}, OtherCycle{},
             OtherCycle{}, "INr"},
            {DCr, FirstCycle{T3(Fetch_IR_And_RegB), T4(IDLE), T5(Dec_Destination)}, OtherCycle{},
             OtherCycle{}, "DCr"},

            // Accumulator Group Instructions
    };

    /*
     * The high part is match exactly.
     * The low and medium part use the 3 high bits as a mask to match the 3 low bits.
     * Eg.
     * - 0b000'000 means 'don't care'
     * - 0b111'101 means the part must be equal to b'101
     */
    decoding_table = {
            {Lrr, 0b11, 0b000'000, 0b000'000}, {LrM, 0b11, 0b000'000, 0b111'111},
            {LMr, 0b11, 0b111'111, 0b000'000}, {LrI, 0b00, 0b000'000, 0b111'110},
            {LMI, 0b00, 0b111'111, 0b111'110}, {INr, 0b00, 0b000'000, 0b111'000},
            {DCr, 0b00, 0b000'000, 0b111'001},
    };
}
constexpr CycleActionsFor8008::T1_Action InstructionTableFor8008::T1(uint8_t actions)
{
    return static_cast<CycleActionsFor8008::T1_Action>(actions);
}
constexpr CycleActionsFor8008::T2_Action InstructionTableFor8008::T2(uint8_t actions)
{
    return static_cast<CycleActionsFor8008::T2_Action>(actions);
}
constexpr CycleActionsFor8008::T3_Action InstructionTableFor8008::T3(uint8_t actions)
{
    return static_cast<CycleActionsFor8008::T3_Action>(actions);
}
constexpr CycleActionsFor8008::T4_Action InstructionTableFor8008::T4(uint8_t actions)
{
    return static_cast<CycleActionsFor8008::T4_Action>(actions);
}
constexpr CycleActionsFor8008::T5_Action InstructionTableFor8008::T5(uint8_t actions)
{
    return static_cast<CycleActionsFor8008::T5_Action>(actions);
}
