#ifndef MICRALN_INSTRUCTIONS8008_H
#define MICRALN_INSTRUCTIONS8008_H

#include "Constants8008.h"

#include <cstdint>
#include <string>
#include <vector>

namespace InstructionNameFor8008
{
    enum InstructionName : std::uint8_t
    {
        // Index Register Instructions
        Lrr,
        LrM,
        LMr,
        LrI,
        LMI,
        INr,
        DCr,
        // Accumulator Group Instructions
        ALU_OPr,
        ALU_OPM,
        ALU_OPI,
        RLC,
        RRC,
        RAL,
        RAR,
        // Program Counter and Stack Control Instructions
        JMP,
        JFc,
        JTc,
        CAL,
        CFc,
        CTc,
        RET,
        RFc,
        RTc,
        RST,
        // I/O Instructions
        INP,
        OUT,
        // Machine Instructions
        HLT,
        // Unknown Instruction
        UNKNOWN,
    };
}

namespace CycleActionsFor8008
{
    const uint8_t IDLE = 32;
    const uint8_t CONDITIONAL_END = 64;
    const uint8_t CYCLE_END = 128;

    enum T1_Action : std::uint8_t
    {
        Out_Reg_L,
        Out_PC_L,
        Out_Reg_A,
    };

    enum T2_Action : std::uint8_t
    {
        Out_Reg_H,
        Out_PC_H,
        Out_Reg_b_At_T2,
    };

    enum T3_Action : std::uint8_t
    {
        Fetch_IR_And_Reg_b = 1,
        Fetch_Data_to_Reg_b = 2,
        Fetch_Data_to_Reg_a = 4,
        Out_Reg_b = 8,
        Halt = 16,
    };

    enum T4_Action : std::uint8_t
    {
        Source_to_Reg_b,
        Reg_a_to_PC_H,
        Push_And_Reg_a_to_PC_H,
        Pop_Stack,
        Out_Conditions_Flags,
    };

    enum T5_Action : std::uint8_t
    {
        Reg_b_to_Destination,
        Inc_Destination,
        Dec_Destination,
        ALU_Operation_With_RegB,
        Rotate_A,
        Reg_b_to_PC_L,
        Reg_b_to_PC_L_3_to_5,
        Reg_b_to_A,
    };

} // namespace CycleActionsFor8008

struct InstructionTableFor8008
{
    struct FirstCycle
    {
        CycleActionsFor8008::T3_Action t3_action;
        CycleActionsFor8008::T4_Action t4_action;
        CycleActionsFor8008::T5_Action t5_action;
    };

    struct OtherCycle
    {
        Constants8008::CycleControl cycle_control;
        CycleActionsFor8008::T1_Action t1_action;
        CycleActionsFor8008::T2_Action t2_action;
        CycleActionsFor8008::T3_Action t3_action;
        CycleActionsFor8008::T4_Action t4_action;
        CycleActionsFor8008::T5_Action t5_action;
    };

    struct Instruction
    {
        InstructionNameFor8008::InstructionName name{};
        FirstCycle cycle_1{};
        OtherCycle cycle_2{};
        OtherCycle cycle_3{};
        std::string opcode_template{};
    };

    struct DecodedInstruction
    {
        const Instruction* instruction;
        uint8_t medium;
        uint8_t low;
    };

    struct Decoding
    {
        InstructionNameFor8008::InstructionName name{};
        uint8_t high_part{};
        uint8_t medium_part{};
        uint8_t low_part{};
    };

    InstructionTableFor8008();

    DecodedInstruction decode_instruction(uint8_t opcode);

private:
    std::vector<Instruction> instruction_table;
    std::vector<Decoding> decoding_table;

    static constexpr CycleActionsFor8008::T1_Action T1(uint8_t actions);
    static constexpr CycleActionsFor8008::T2_Action T2(uint8_t actions);
    static constexpr CycleActionsFor8008::T3_Action T3(uint8_t actions);
    static constexpr CycleActionsFor8008::T4_Action T4(uint8_t actions);
    static constexpr CycleActionsFor8008::T5_Action T5(uint8_t actions);
};

std::string instruction_to_string(InstructionTableFor8008::DecodedInstruction & instruction);

#endif //MICRALN_INSTRUCTIONS8008_H
