#include "Disassemble8008.h"
#include "MemoryView.h"

#include <misc_utils/src/ToHex.h>

namespace
{
    const std::string DECODING_ERROR{"???"};
}

Disassemble8008::Disassemble8008(const MemoryView& memory_view) : memory_view(memory_view) {}

std::tuple<std::string, size_t> Disassemble8008::get_as_string(uint16_t address)
{
    auto [text_opcode, operand, size] = get_extended(address);
    if (!operand.empty())
    {
        text_opcode.append(" ").append(operand);
    }
    return {text_opcode, size};
}

std::tuple<std::string, std::string, std::uint16_t> Disassemble8008::get_extended(uint16_t address)
{
    auto decoded = instruction_table.decode_instruction(memory_view.get(address));

    if (decoded.instruction->name == InstructionNameFor8008::UNKNOWN)
    {
        return {DECODING_ERROR, "", 1};
    }

    auto text_opcode = instruction_to_string(decoded);

    switch (decoded.instruction->name)
    {
        case InstructionNameFor8008::LrI:
        case InstructionNameFor8008::LMI:
        case InstructionNameFor8008::ALU_OPI: {
            auto value_address = static_cast<size_t>(address + 1);
            if (value_address >= memory_view.size())
            {
                return {DECODING_ERROR, "", 1};
            }
            auto immediate_value = memory_view.get(value_address);

            return {text_opcode, utils::to_hex<int, 2>(immediate_value), 2};
        }
        default:;
    }

    switch (decoded.instruction->name)
    {
        case InstructionNameFor8008::JMP:
        case InstructionNameFor8008::JFc:
        case InstructionNameFor8008::JTc:
        case InstructionNameFor8008::CAL:
        case InstructionNameFor8008::CFc:
        case InstructionNameFor8008::CTc: {
            auto value_address = static_cast<size_t>(address + 2);
            if (value_address >= memory_view.size())
            {
                return {DECODING_ERROR, "", 1};
            }
            auto immediate_value =
                    memory_view.get(value_address) << 8 | memory_view.get(value_address - 1);

            return {text_opcode, utils::to_hex<int, 4>(immediate_value), 3};
        }
        default:;
    }

    std::string operand;
    if (decoded.instruction->name == InstructionNameFor8008::RST)
    {
        int rst_address = (decoded.medium << 3);
        operand = utils::to_hex<int, 2>(rst_address);
    }
    else if (decoded.instruction->name == InstructionNameFor8008::INP)
    {
        int device_address = (memory_view.get(address) & 0b00001110) >> 1;
        operand = utils::to_hex<int, 1>(device_address);
    }
    else if (decoded.instruction->name == InstructionNameFor8008::OUT)
    {
        int device_address = ((memory_view.get(address) & 0b00111110) >> 1) - 8;
        operand = utils::to_hex<int, 1>(device_address);
    }
    else if (decoded.instruction->name == InstructionNameFor8008::RET)
    {
        int middle = decoded.medium & 0b111;
        if (middle == 3)
        {
            text_opcode = "REI";
        }
    }
    else if (text_opcode == "LLL")
    {
        text_opcode = "DMS";
    }
    else if (text_opcode == "LCC")
    {
        text_opcode = "MAS";
    }

    return {text_opcode, operand, 1};
}
