#ifndef MICRALN_DISASSEMBLE8008_H
#define MICRALN_DISASSEMBLE8008_H

#include "Instructions8008.h"

#include <cstdint>
#include <span>
#include <string>

class Disassemble8008
{
public:
    explicit Disassemble8008(const std::span<std::uint8_t>& data);
    std::tuple<std::string, size_t> get(uint16_t address);

private:
    InstructionTableFor8008 instruction_table;
    const std::span<std::uint8_t>& data;
};

#endif //MICRALN_DISASSEMBLE8008_H