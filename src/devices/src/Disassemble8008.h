#ifndef MICRALN_DISASSEMBLE8008_H
#define MICRALN_DISASSEMBLE8008_H

#include "Instructions8008.h"

#include <cstdint>
#include <span>
#include <string>

class MemoryView;

class Disassemble8008
{
public:
    explicit Disassemble8008(const MemoryView & memory_view);

    std::tuple<std::string, size_t> get(uint16_t address);

private:
    InstructionTableFor8008 instruction_table;
    const MemoryView & memory_view;
};

#endif //MICRALN_DISASSEMBLE8008_H
