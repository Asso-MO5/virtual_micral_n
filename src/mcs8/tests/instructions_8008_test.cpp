#include "Instructions8008.h"

#include "gmock/gmock.h"

using namespace testing;

InstructionNameFor8008::InstructionName decode(InstructionTableFor8008& table, uint8_t opcode)
{
    return table.decode_instruction(opcode).instruction->name;
}

TEST(Instruction8008, can_decode_index_register_instructions)
{
    InstructionTableFor8008 table;

    ASSERT_THAT(decode(table, 0b11'000'000), Eq(InstructionNameFor8008::Lrr));
    ASSERT_THAT(decode(table, 0b11'000'111), Eq(InstructionNameFor8008::LrM));
    ASSERT_THAT(decode(table, 0b11'111'000), Eq(InstructionNameFor8008::LMr));
    ASSERT_THAT(decode(table, 0b00'000'110), Eq(InstructionNameFor8008::LrI));
    ASSERT_THAT(decode(table, 0b00'111'110), Eq(InstructionNameFor8008::LMI));
    ASSERT_THAT(decode(table, 0b00'001'000), Eq(InstructionNameFor8008::INr));
    ASSERT_THAT(decode(table, 0b00'001'001), Eq(InstructionNameFor8008::DCr));
}

TEST(Instruction8008, can_decode_accumulator_group_instructions)
{
    InstructionTableFor8008 table;

    ASSERT_THAT(decode(table, 0b10'000'111), Eq(InstructionNameFor8008::ALU_OPM));
    ASSERT_THAT(decode(table, 0b10'000'000), Eq(InstructionNameFor8008::ALU_OPr));
    ASSERT_THAT(decode(table, 0b00'000'100), Eq(InstructionNameFor8008::ALU_OPI));
    ASSERT_THAT(decode(table, 0b00'000'010), Eq(InstructionNameFor8008::RLC));
    ASSERT_THAT(decode(table, 0b00'001'010), Eq(InstructionNameFor8008::RRC));
    ASSERT_THAT(decode(table, 0b00'010'010), Eq(InstructionNameFor8008::RAL));
    ASSERT_THAT(decode(table, 0b00'011'010), Eq(InstructionNameFor8008::RAR));
}

TEST(Instruction8008, can_decode_pc_and_stack_control_instructions)
{
    InstructionTableFor8008 table;

    ASSERT_THAT(decode(table, 0b01'000'100), Eq(InstructionNameFor8008::JMP));
    ASSERT_THAT(decode(table, 0b01'000'000), Eq(InstructionNameFor8008::JFc));
    ASSERT_THAT(decode(table, 0b01'100'000), Eq(InstructionNameFor8008::JTc));
    ASSERT_THAT(decode(table, 0b01'000'110), Eq(InstructionNameFor8008::CAL));
    ASSERT_THAT(decode(table, 0b01'000'010), Eq(InstructionNameFor8008::CFc));
    ASSERT_THAT(decode(table, 0b01'100'010), Eq(InstructionNameFor8008::CTc));
    ASSERT_THAT(decode(table, 0b00'000'111), Eq(InstructionNameFor8008::RET));
    ASSERT_THAT(decode(table, 0b00'000'011), Eq(InstructionNameFor8008::RFc));
    ASSERT_THAT(decode(table, 0b00'100'011), Eq(InstructionNameFor8008::RTc));
    ASSERT_THAT(decode(table, 0b00'000'101), Eq(InstructionNameFor8008::RST));
}

TEST(Instruction8008, can_decode_io_instructions)
{
    InstructionTableFor8008 table;

    ASSERT_THAT(decode(table, 0b01'000'001), Eq(InstructionNameFor8008::INP));
    ASSERT_THAT(decode(table, 0b01'110'001), Eq(InstructionNameFor8008::OUT));
}

TEST(Instruction8008, can_decode_halt_instructions)
{
    InstructionTableFor8008 table;

    ASSERT_THAT(decode(table, 0b00'000'000), Eq(InstructionNameFor8008::HLT));
    ASSERT_THAT(decode(table, 0b00'000'001), Eq(InstructionNameFor8008::HLT));
    ASSERT_THAT(decode(table, 0b11'111'111), Eq(InstructionNameFor8008::HLT));
}
