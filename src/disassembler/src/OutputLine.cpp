#include "OutputLine.h"

#include <file_utils/src/FileReader.h>
#include <mcs8/src/Disassemble8008.h>
#include <mcs8/src/MemoryView.h>
#include <misc_utils/src/ToHex.h>

#include <iomanip>
#include <iostream>

OutputLine::OutputLine(const Disassemble8008& disassembler, const MemoryView& rom_view,
                       uint16_t address, std::uint16_t origin_address)
    : rom_view{rom_view}, address{address}, origin_address{origin_address}
{
    auto [dec_instruction, dec_operand, dec_size] = disassembler.get_extended(address);

    instruction = std::move(dec_instruction);
    operand = std::move(dec_operand);
    size = dec_size;
}

bool OutputLine::is_end_of_block_instruction() const
{
    return instruction == "JMP" || instruction == "RET" || instruction == "REI";
}
void OutputLine::output_line(std::ostream& os) const
{
    output_address(os);
    output_bytes(os);
    output_instruction(os);
    output_operand(os);
}

void OutputLine::output_address(std::ostream& os) const
{
    auto address_as_hex_string = utils::to_hex<uint16_t, 4>(address + origin_address);
    os << std::setfill(' ') << std::setw(8) << std::left << address_as_hex_string;
}

std::stringstream OutputLine::get_bytes() const
{
    std::stringstream bytes;
    for (uint16_t sub_address = address; sub_address < address + size; sub_address += 1)
    {
        bytes << std::setfill(' ') << std::setw(4) << std::left;
        bytes << utils::to_hex<uint8_t, 2>(rom_view.get(sub_address));
    }
    return bytes;
}

void OutputLine::output_bytes(std::ostream& os) const
{
    std::stringstream bytes = get_bytes();
    os << std::setfill(' ') << std::setw(16) << std::left << bytes.str();
}

void OutputLine::output_instruction(std::ostream& os) const { os << instruction.c_str(); }

void OutputLine::output_operand(std::ostream& os) const
{
    if (!operand.empty())
    {
        os << " " << operand.c_str();
    }
}

std::ostream& operator<<(std::ostream& os, const OutputLine& line)
{
    line.output_line(os);
    return os;
}

uint16_t OutputLine::get_instruction_size() const { return size; }
