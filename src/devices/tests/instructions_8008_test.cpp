#include "Instructions8008.h"

#include "gmock/gmock.h"

using namespace testing;

InstructionNameFor8008::InstructionName decode(InstructionTableFor8008& table, uint8_t opcode)
{
    return table.decode_instruction(opcode).instruction.name;
}

TEST(Instruction8008, can_decode_index_register_instructions)
{
    InstructionTableFor8008 table;

    ASSERT_THAT(decode(table, 0b11'000'000), Eq(InstructionNameFor8008::Lrr));
    ASSERT_THAT(decode(table, 0b11'000'111), Eq(InstructionNameFor8008::LrM));
    ASSERT_THAT(decode(table, 0b11'111'000), Eq(InstructionNameFor8008::LMr));
    ASSERT_THAT(decode(table, 0b00'000'110), Eq(InstructionNameFor8008::LrI));
    ASSERT_THAT(decode(table, 0b00'111'110), Eq(InstructionNameFor8008::LMI));
    ASSERT_THAT(decode(table, 0b00'000'000), Eq(InstructionNameFor8008::INr));
    ASSERT_THAT(decode(table, 0b00'000'001), Eq(InstructionNameFor8008::DCr));
}
