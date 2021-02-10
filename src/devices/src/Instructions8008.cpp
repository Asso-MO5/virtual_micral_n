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

    /* Comments are there to force clang format to a more readable format */
    instruction_table = {
            // Index Register Instructions
            {Lrr, // name
             FirstCycle{T3(Fetch_IR_And_RegB), T4(Source_to_RegB),
                        T5(RegB_to_Destination)}, // 1st Cycle
             OtherCycle{}, OtherCycle{}, "Lrr"},
            {LrM,                                           // name
             FirstCycle{T3(Fetch_IR_And_RegB | CYCLE_END)}, // 1st Cycle
             OtherCycle{CPU8008::CycleControl::PCR, T1(Out_Reg_L), T2(Out_Reg_H),
                        T3(Fetch_Data_to_RegB), T4(IDLE), T5(RegB_to_Destination)}, // 2nd Cycle
             OtherCycle{}, "LrM"},
            {LMr,                                                               // name
             FirstCycle{T3(Fetch_IR_And_RegB), T4(Source_to_RegB | CYCLE_END)}, // 1st Cycle
             OtherCycle{CPU8008::CycleControl::PCW, T1(Out_Reg_L), T2(Out_Reg_H),
                        T3(Out_RegB | CYCLE_END)}, // 2nd Cycle
             OtherCycle{}, "LMr"},
            {LrI,                                           // name
             FirstCycle{T3(Fetch_IR_And_RegB | CYCLE_END)}, // 1st Cycle
             OtherCycle{CPU8008::CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),
                        T3(Fetch_Data_to_RegB), T4(IDLE), T5(RegB_to_Destination)}, // 2nd Cycle
             OtherCycle{}, "LrI"},
            {LMI,                                           // name
             FirstCycle{T3(Fetch_IR_And_RegB | CYCLE_END)}, // 1st Cycle
             OtherCycle{CPU8008::CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),
                        T3(Fetch_Data_to_RegB | CYCLE_END)}, // 2nd Cycle
             OtherCycle{CPU8008::CycleControl::PCW, T1(Out_Reg_L), T2(Out_Reg_H),
                        T3(Out_RegB | CYCLE_END)},
             "LMI"},
            {INr,                                                              // name
             FirstCycle{T3(Fetch_IR_And_RegB), T4(IDLE), T5(Inc_Destination)}, // 1st Cycle
             OtherCycle{}, OtherCycle{}, "INr"},
            {DCr,                                                              // name
             FirstCycle{T3(Fetch_IR_And_RegB), T4(IDLE), T5(Dec_Destination)}, // 1st Cycle
             OtherCycle{}, OtherCycle{}, "DCr"},

            // Accumulator Group Instructions
            {ALU_OPr, // name
             FirstCycle{T3(Fetch_IR_And_RegB), T4(Source_to_RegB), T5(ALU_Operation_With_RegB)},
             OtherCycle{}, OtherCycle{}, "ppr"},
            {ALU_OPM, // name
             FirstCycle{T3(Fetch_IR_And_RegB | CYCLE_END)},
             OtherCycle{CPU8008::CycleControl::PCR, T1(Out_Reg_L), T2(Out_Reg_H),
                        T3(Fetch_Data_to_RegB), T4(IDLE), T5(ALU_Operation_With_RegB)}, // 2nd Cycle
             OtherCycle{}, "ppM"},
            {ALU_OPI, // name
             FirstCycle{T3(Fetch_IR_And_RegB | CYCLE_END)},
             OtherCycle{CPU8008::CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),
                        T3(Fetch_Data_to_RegB), T4(IDLE), T5(ALU_Operation_With_RegB)}, // 2nd Cycle
             OtherCycle{}, "ppI"},
            {RLC, // name
             FirstCycle{T3(Fetch_IR_And_RegB), T4(IDLE), T5(Rotate_A)}, OtherCycle{}, OtherCycle{},
             "RLC"},
            {RRC, // name
             FirstCycle{T3(Fetch_IR_And_RegB), T4(IDLE), T5(Rotate_A)}, OtherCycle{}, OtherCycle{},
             "RRC"},
            {RAL, // name
             FirstCycle{T3(Fetch_IR_And_RegB), T4(IDLE), T5(Rotate_A)}, OtherCycle{}, OtherCycle{},
             "RAL"},
            {RAR, // name
             FirstCycle{T3(Fetch_IR_And_RegB), T4(IDLE), T5(Rotate_A)}, OtherCycle{}, OtherCycle{},
             "RAC"},
    };

    /*
     * The high part is match exactly.
     * The low and medium part use the 3 high bits as a mask to match the 3 low bits.
     * Eg.
     * - 0b000'000 means 'don't care'
     * - 0b111'101 means the part must be equal to b'101
     */
    decoding_table = {
            // Index Register Instructions
            {Lrr, 0b11, 0b000'000, 0b000'000},     //
            {LrM, 0b11, 0b000'000, 0b111'111},     //
            {LMr, 0b11, 0b111'111, 0b000'000},     //
            {LrI, 0b00, 0b000'000, 0b111'110},     //
            {LMI, 0b00, 0b111'111, 0b111'110},     //
            {INr, 0b00, 0b000'000, 0b111'000},     //
            {DCr, 0b00, 0b000'000, 0b111'001},     //
                                                   // Accumulator Group Instructions
            {ALU_OPr, 0b10, 0b000'000, 0b000'000}, //
            {ALU_OPM, 0b10, 0b000'000, 0b111'111}, //
            {ALU_OPI, 0b00, 0b000'000, 0b111'100}, //
            {RLC, 0b00, 0b111'000, 0b111'010},     //
            {RRC, 0b00, 0b111'001, 0b111'010},     //
            {RAL, 0b00, 0b111'010, 0b111'010},     //
            {RAR, 0b00, 0b111'011, 0b111'010},     //
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
