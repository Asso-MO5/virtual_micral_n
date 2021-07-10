#include "MemoryPage.h"

#include <cassert>

MemoryPage::~MemoryPage() =default;

ActiveMemoryPage::ActiveMemoryPage(std::span<std::uint8_t> buffer) : buffer{buffer} {}

void ActiveMemoryPage::write(std::uint16_t address, std::uint8_t data)
{
    assert(address < buffer.size() && "Not a valid address for this page");
    buffer[address] = data;
}

std::uint8_t ActiveMemoryPage::read(std::uint16_t address) const
{
    assert(address < buffer.size() && "Not a valid address for this page");
    return buffer[address];
}

void InactiveMemoryPage::write(std::uint16_t address, std::uint8_t data) {}

std::uint8_t InactiveMemoryPage::read(std::uint16_t address) const { return 0; }
