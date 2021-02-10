#ifndef MICRALN_INSTRUCTIONS8008_H
#define MICRALN_INSTRUCTIONS8008_H

#include "CPU8008.h"
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
    const uint8_t IDLE = 64;
    const uint8_t CYCLE_END = 128;

    enum T1_Action : std::uint8_t
    {
        Out_Reg_L,
        Out_PC_L,
    };

    enum T2_Action : std::uint8_t
    {
        Out_Reg_H,
        Out_PC_H,
    };

    enum T3_Action : std::uint8_t
    {
        Fetch_IR_And_RegB = 1,
        Fetch_Data_to_RegB = 2,
        Push_Stack = 4,
        Condition = 8,
        Out_RegB = 16,
        Halt = 32,
    };

    enum T4_Action : std::uint8_t
    {
        Source_to_RegB
    };

    enum T5_Action : std::uint8_t
    {
        RegB_to_Destination,
        Inc_Destination,
        Dec_Destination,
        ALU_Operation_With_RegB,
        Rotate_A,
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
        CPU8008::CycleControl cycle_control;
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
        const Instruction& instruction;
        uint8_t medium;
        uint8_t low;
    };

    struct Decoding
    {
        InstructionNameFor8008::InstructionName name{};
        uint8_t high_part;
        uint8_t medium_part;
        uint8_t low_part;
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

#endif //MICRALN_INSTRUCTIONS8008_H
