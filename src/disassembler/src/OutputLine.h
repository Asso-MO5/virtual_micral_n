#ifndef MICRALN_OUTPUTLINE_H
#define MICRALN_OUTPUTLINE_H

#include <iostream>

class Disassemble8008;
class MemoryView;

class OutputLine
{
public:
    OutputLine(const Disassemble8008& disassembler, const MemoryView& rom_view, std::uint16_t address);

    [[nodiscard]] bool is_end_of_block_instruction() const;
    [[nodiscard]] uint16_t get_instruction_size() const;

    friend std::ostream& operator<<(std::ostream& os, const OutputLine& line);

private:
    void output_line(std::ostream& os) const;
    void output_address(std::ostream& os) const;
    [[nodiscard]] std::stringstream get_bytes() const;
    void output_bytes(std::ostream& os) const;
    void output_instruction(std::ostream& os) const;
    void output_operand(std::ostream& os) const;

    const MemoryView& rom_view;
    std::string instruction;
    std::string operand;
    uint16_t size;
    uint16_t address;
};

#endif //MICRALN_OUTPUTLINE_H
