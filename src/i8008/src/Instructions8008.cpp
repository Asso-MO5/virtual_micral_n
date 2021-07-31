#include "Instructions8008.h"

#include <cassert>

using namespace Constants8008;

InstructionTableFor8008::DecodedInstruction
InstructionTableFor8008::decode_instruction(uint8_t opcode) const
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
        auto mask_low = (decode_low & 0b111000) >> 3;
        auto result_low = low_part & mask_low;
        if (result_low != (decode_low & 0b111))
        {
            continue;
        }
        auto mask_medium = (decode_medium & 0b111000) >> 3;
        auto result_medium = medium_part & mask_medium;
        if (result_medium != (decode_medium & 0b111))
        {
            continue;
        }

        if (candidate == InstructionNameFor8008::UNKNOWN)
        {
            return {&unknown_instruction, 0, 0};
        }

        auto& found = instruction_table[candidate];
        assert(found.name == candidate); // Both table must be synchronized

        return {&found, medium_part, low_part};
    }
    return {&unknown_instruction, 0, 0};
}

InstructionTableFor8008::InstructionTableFor8008()
{
    using namespace InstructionNameFor8008;
    using namespace CycleActionsFor8008;

    /* Comments are there to force clang format to a more readable format */
    instruction_table = {
            // Index Register Instructions
            {Lrr,                                                                               //
             FirstCycle{T3(Fetch_IR_And_Reg_b), T4(Source_to_Reg_b), T5(Reg_b_to_Destination)}, //
             OtherCycle{}, OtherCycle{}, "Lrr"},                                                //
            {LrM,                                                                               //
             FirstCycle{T3(Fetch_IR_And_Reg_b | CYCLE_END)},                                    //
             OtherCycle{CycleControl::PCR, T1(Out_Reg_L), T2(Out_Reg_H),                        //
                        T3(Fetch_Data_to_Reg_b), T4(IDLE), T5(Reg_b_to_Destination)},           //
             OtherCycle{}, "LrM"},                                                              //
            {LMr,                                                                               //
             FirstCycle{T3(Fetch_IR_And_Reg_b), T4(Source_to_Reg_b | CYCLE_END)},               //
             OtherCycle{CycleControl::PCW, T1(Out_Reg_L), T2(Out_Reg_H),                        //
                        T3(Out_Reg_b | CYCLE_END)},                                             //
             OtherCycle{}, "LMr"},                                                              //
            {LrI,                                                                               //
             FirstCycle{T3(Fetch_IR_And_Reg_b | CYCLE_END)},                                    //
             OtherCycle{CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),                          //
                        T3(Fetch_Data_to_Reg_b), T4(IDLE), T5(Reg_b_to_Destination)},           //
             OtherCycle{}, "LrI"},                                                              //
            {LMI,                                                                               //
             FirstCycle{T3(Fetch_IR_And_Reg_b | CYCLE_END)},                                    //
             OtherCycle{CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),                          //
                        T3(Fetch_Data_to_Reg_b | CYCLE_END)},                                   //
             OtherCycle{CycleControl::PCW, T1(Out_Reg_L), T2(Out_Reg_H),                        //
                        T3(Out_Reg_b | CYCLE_END)},                                             //
             "LMI"},                                                                            //
            {INr,                                                                               //
             FirstCycle{T3(Fetch_IR_And_Reg_b), T4(IDLE), T5(Inc_Destination)},                 //
             OtherCycle{}, OtherCycle{}, "INr"},                                                //
            {DCr,                                                                               //
             FirstCycle{T3(Fetch_IR_And_Reg_b), T4(IDLE), T5(Dec_Destination)},                 //
             OtherCycle{}, OtherCycle{}, "DCr"},                                                //

            // Accumulator Group Instructions
            {ALU_OPr,                                                                    //
             FirstCycle{T3(Fetch_IR_And_Reg_b), T4(Source_to_Reg_b),                     //
                        T5(ALU_Operation_With_RegB)},                                    //
             OtherCycle{}, OtherCycle{}, "ppr"},                                         //
            {ALU_OPM,                                                                    //
             FirstCycle{T3(Fetch_IR_And_Reg_b | CYCLE_END)},                             //
             OtherCycle{CycleControl::PCR, T1(Out_Reg_L), T2(Out_Reg_H),                 //
                        T3(Fetch_Data_to_Reg_b), T4(IDLE), T5(ALU_Operation_With_RegB)}, //
             OtherCycle{}, "ppM"},                                                       //
            {ALU_OPI,                                                                    //
             FirstCycle{T3(Fetch_IR_And_Reg_b | CYCLE_END)},                             //
             OtherCycle{CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),                   //
                        T3(Fetch_Data_to_Reg_b), T4(IDLE), T5(ALU_Operation_With_RegB)}, //
             OtherCycle{}, "ppI"},                                                       //
            {RLC,                                                                        //
             FirstCycle{T3(Fetch_IR_And_Reg_b), T4(IDLE), T5(Rotate_A)},                 //
             OtherCycle{}, OtherCycle{}, "RLC"},                                         //
            {RRC,                                                                        //
             FirstCycle{T3(Fetch_IR_And_Reg_b), T4(IDLE), T5(Rotate_A)},                 //
             OtherCycle{}, OtherCycle{}, "RRC"},                                         //
            {RAL,                                                                        //
             FirstCycle{T3(Fetch_IR_And_Reg_b), T4(IDLE), T5(Rotate_A)},                 //
             OtherCycle{}, OtherCycle{}, "RAL"},                                         //
            {RAR,                                                                        //
             FirstCycle{T3(Fetch_IR_And_Reg_b), T4(IDLE), T5(Rotate_A)},                 //
             OtherCycle{}, OtherCycle{}, "RAC"},                                         //

            // Program Counter and Stack Control Instructions
            {JMP, FirstCycle{T3(Fetch_IR_And_Reg_b | CYCLE_END)},                                //
             OtherCycle{CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),                           //
                        T3(Fetch_Data_to_Reg_b | CYCLE_END)},                                    //
             OtherCycle{CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),                           //
                        T3(Fetch_Data_to_Reg_a), T4(Reg_a_to_PC_H), T5(Reg_b_to_PC_L)},          //
             "JMP"},                                                                             //
            {JFc, FirstCycle{T3(Fetch_IR_And_Reg_b | CYCLE_END)},                                //
             OtherCycle{CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),                           //
                        T3(Fetch_Data_to_Reg_b | CYCLE_END)},                                    //
             OtherCycle{CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),                           //
                        T3(Fetch_Data_to_Reg_a | CONDITIONAL_END), T4(Reg_a_to_PC_H),            //
                        T5(Reg_b_to_PC_L)},                                                      //
             "JFc"},                                                                             //
            {JTc, FirstCycle{T3(Fetch_IR_And_Reg_b | CYCLE_END)},                                //
             OtherCycle{CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),                           //
                        T3(Fetch_Data_to_Reg_b | CYCLE_END)},                                    //
             OtherCycle{CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),                           //
                        T3(Fetch_Data_to_Reg_a | CONDITIONAL_END), T4(Reg_a_to_PC_H),            //
                        T5(Reg_b_to_PC_L)},                                                      //
             "JTc"},                                                                             //
            {CAL, FirstCycle{T3(Fetch_IR_And_Reg_b | CYCLE_END)},                                //
             OtherCycle{CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),                           //
                        T3(Fetch_Data_to_Reg_b | CYCLE_END)},                                    //
             OtherCycle{CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),                           //
                        T3(Fetch_Data_to_Reg_a), T4(Push_And_Reg_a_to_PC_H), T5(Reg_b_to_PC_L)}, //
             "CAL"},                                                                             //
            {CFc, FirstCycle{T3(Fetch_IR_And_Reg_b | CYCLE_END)},                                //
             OtherCycle{CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),                           //
                        T3(Fetch_Data_to_Reg_b | CYCLE_END)},                                    //
             OtherCycle{CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H),                           //
                        T3(Fetch_Data_to_Reg_a | CONDITIONAL_END), T4(Push_And_Reg_a_to_PC_H),
                        T5(Reg_b_to_PC_L)},                            //
             "CFc"},                                                   //
            {CTc, FirstCycle{T3(Fetch_IR_And_Reg_b | CYCLE_END)},      //
             OtherCycle{CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H), //
                        T3(Fetch_Data_to_Reg_b | CYCLE_END)},          //
             OtherCycle{CycleControl::PCR, T1(Out_PC_L), T2(Out_PC_H), //
                        T3(Fetch_Data_to_Reg_a | CONDITIONAL_END), T4(Push_And_Reg_a_to_PC_H),
                        T5(Reg_b_to_PC_L)},                                                      //
             "CTc"},                                                                             //
            {RET, FirstCycle{T3(Fetch_IR_And_Reg_b), T4(Pop_Stack), T5(IDLE)},                   //
             OtherCycle{}, OtherCycle{}, "RET"},                                                 //
            {RFc, FirstCycle{T3(Fetch_IR_And_Reg_b | CONDITIONAL_END), T4(Pop_Stack), T5(IDLE)}, //
             OtherCycle{}, OtherCycle{}, "RFc"},                                                 //
            {RTc, FirstCycle{T3(Fetch_IR_And_Reg_b | CONDITIONAL_END), T4(Pop_Stack), T5(IDLE)}, //
             OtherCycle{}, OtherCycle{}, "RTc"},                                                 //
            {RST,
             FirstCycle{T3(Fetch_IR_And_Reg_b), T4(Push_And_Reg_a_to_PC_H), //
                        T5(Reg_b_to_PC_L_3_to_5)},                          //
             OtherCycle{}, OtherCycle{}, "RST"},                            //

            // I/O Instructions
            {INP, FirstCycle{T3(Fetch_IR_And_Reg_b | CYCLE_END)},                           //
             OtherCycle{CycleControl::PCC, T1(Out_Reg_A), T2(Out_Reg_b_At_T2),              //
                        T3(Fetch_Data_to_Reg_b), T4(Out_Conditions_Flags), T5(Reg_b_to_A)}, //
             OtherCycle{}, "INP"},                                                          //
            {OUT, FirstCycle{T3(Fetch_IR_And_Reg_b | CYCLE_END)},                           //
             OtherCycle{CycleControl::PCC, T1(Out_Reg_A), T2(Out_Reg_b_At_T2),
                        T3(IDLE | CYCLE_END)}, //
             OtherCycle{}, "OUT"},             //

            // HALT
            {HLT, FirstCycle{static_cast<T3_Action>(Fetch_IR_And_Reg_b | Halt | CYCLE_END)}, //
             OtherCycle{}, OtherCycle{}, "HLT"}                                              //
    };

    /*
     * The high part is match exactly.
     * The low and medium part use the 3 high bits as a mask to match the 3 low bits.
     * Eg.
     * - 0b000'000 means 'don't care'
     * - 0b111'101 means the part must be equal to b'101
     */
    decoding_table = {
            // HALT
            {HLT, 0b00, 0b111'000, 0b110'000}, // Must be before INr to capture the illegal INA
            {HLT, 0b11, 0b111'111, 0b111'111}, //

            // Index Register Instructions
            {LrM, 0b11, 0b000'000, 0b111'111},     // Must be before Lrr to capture M
            {LMr, 0b11, 0b111'111, 0b000'000},     // Must be before Lrr to capture M
            {Lrr, 0b11, 0b000'000, 0b000'000},     //
            {LMI, 0b00, 0b111'111, 0b111'110},     // Must be before LrI to capture M
            {LrI, 0b00, 0b000'000, 0b111'110},     //
            {UNKNOWN, 0b00, 0b111'111, 0b111'000}, // Captures the invalid Increment M
            {UNKNOWN, 0b00, 0b111'111, 0b111'001}, // Captures the invalid Decrement M
            {INr, 0b00, 0b000'000, 0b111'000},     //
            {DCr, 0b00, 0b000'000, 0b111'001},     //

            // Accumulator Group Instructions
            {ALU_OPM, 0b10, 0b000'000, 0b111'111}, // Must be before ALU_OPr to capture M
            {ALU_OPr, 0b10, 0b000'000, 0b000'000}, //
            {ALU_OPI, 0b00, 0b000'000, 0b111'100}, //
            {RLC, 0b00, 0b111'000, 0b111'010},     //
            {RRC, 0b00, 0b111'001, 0b111'010},     //
            {RAL, 0b00, 0b111'010, 0b111'010},     //
            {RAR, 0b00, 0b111'011, 0b111'010},     //

            // Program Counter and Stack Control Instructions
            {JMP, 0b01, 0b000'000, 0b111'100}, //
            {JFc, 0b01, 0b100'000, 0b111'000}, //
            {JTc, 0b01, 0b100'100, 0b111'000}, //
            {CAL, 0b01, 0b000'000, 0b111'110}, //
            {CFc, 0b01, 0b100'000, 0b111'010}, //
            {CTc, 0b01, 0b100'100, 0b111'010}, //
            {RET, 0b00, 0b000'000, 0b111'111}, //
            {RFc, 0b00, 0b100'000, 0b111'011}, //
            {RTc, 0b00, 0b100'100, 0b111'011}, //
            {RST, 0b00, 0b000'000, 0b111'101}, //

            // I/O Instructions
            {INP, 0b01, 0b110'000, 0b001'001}, //
            {OUT, 0b01, 0b000'000, 0b001'001}, //
    };
}

std::string instruction_to_string(const InstructionTableFor8008::DecodedInstruction& instruction)
{
    if (instruction.instruction == nullptr)
    {
        return {"---"};
    }
    std::string base_string{instruction.instruction->opcode_template};

    if (base_string[1] == 'r')
    {
        base_string[1] = REGISTER_NAMES[instruction.medium][0];
    }
    if (base_string[2] == 'r')
    {
        if (instruction.instruction->name == InstructionNameFor8008::InstructionName::INr ||
            instruction.instruction->name == InstructionNameFor8008::InstructionName::DCr)
        {
            base_string[2] = REGISTER_NAMES[instruction.medium][0];
        }
        else
        {
            base_string[2] = REGISTER_NAMES[instruction.low][0];
        }
    }
    if (base_string[2] == 'c')
    {
        base_string[2] = FLAG_NAMES[instruction.medium & 0b11][0];
    }
    if (base_string[0] == 'p' && base_string[1] == 'p')
    {
        static const char* OPERATION_NAMES[] = {"AD", "AC", "SU", "SB", "ND", "XR", "OR", "CP"};
        base_string[0] = OPERATION_NAMES[instruction.medium][0];
        base_string[1] = OPERATION_NAMES[instruction.medium][1];
    }

    return base_string;
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
