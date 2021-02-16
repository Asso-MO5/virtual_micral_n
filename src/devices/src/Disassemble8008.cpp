#include "Disassemble8008.h"
#include "MemoryView.h"

#include <iomanip>

namespace
{
    const std::string DECODING_ERROR{"???"};

    template<typename IntType, size_t padding>
    std::string to_hex(IntType value)
    {
        std::stringstream hex_value;
        hex_value << " $" << std::setfill('0') << std::setw(padding) << std::hex
                  << static_cast<int>(value);

        return hex_value.str();
    }

} // namespace

Disassemble8008::Disassemble8008(const MemoryView& memory_view) : memory_view(memory_view) {}

std::tuple<std::string, size_t> Disassemble8008::get(uint16_t address)
{
    auto decoded = instruction_table.decode_instruction(memory_view.get(address));

    if (decoded.instruction->name == InstructionNameFor8008::UNKNOWN)
    {
        return {DECODING_ERROR, 1};
    }

    auto text_opcode = instruction_to_string(decoded);

    switch (decoded.instruction->name)
    {
        case InstructionNameFor8008::LrI:
        case InstructionNameFor8008::LMI:
        case InstructionNameFor8008::ALU_OPI: {
            auto value_address = address + 1;
            if (value_address >= memory_view.size())
            {
                return {DECODING_ERROR, 1};
            }
            auto immediate_value = memory_view.get(value_address);

            text_opcode.append(to_hex<int, 2>(immediate_value));

            return {text_opcode, 2};
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
            auto value_address = address + 2;
            if (value_address >= memory_view.size())
            {
                return {DECODING_ERROR, 1};
            }
            auto immediate_value =
                    memory_view.get(value_address) << 8 | memory_view.get(value_address - 1);

            text_opcode.append(to_hex<int, 4>(immediate_value));

            return {text_opcode, 3};
        }
        default:;
    }

    if (decoded.instruction->name == InstructionNameFor8008::RST)
    {
        int rst_address = (decoded.medium << 3);
        text_opcode.append(to_hex<int, 2>(rst_address));
    }
    else if (decoded.instruction->name == InstructionNameFor8008::INP)
    {
        int device_address = (memory_view.get(address) & 0b00001110) >> 1;
        text_opcode.append(to_hex<int, 1>(device_address));
    }

    else if (decoded.instruction->name == InstructionNameFor8008::OUT)
    {
        int device_address = ((memory_view.get(address) & 0b00111110) >> 1) - 8;
        text_opcode.append(to_hex<int, 1>(device_address));
    }

    return {text_opcode, 1};
}
