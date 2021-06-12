#ifndef MICRALN_DISASSEMBLE8008_H
#define MICRALN_DISASSEMBLE8008_H

#include "Instructions8008.h"

#include <cstdint>
#include <span>
#include <string>
#include <tuple>

class MemoryView;

class Disassemble8008
{
public:
    explicit Disassemble8008(const MemoryView & memory_view);

    std::tuple<std::string, size_t> get_as_string(uint16_t address);
    std::tuple<std::string, std::string, std::uint16_t> get_extended(uint16_t address);

private:
    InstructionTableFor8008 instruction_table;
    const MemoryView & memory_view;
};

#endif //MICRALN_DISASSEMBLE8008_H
