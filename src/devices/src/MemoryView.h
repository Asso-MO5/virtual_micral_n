#ifndef MICRALN_MEMORYVIEW_H
#define MICRALN_MEMORYVIEW_H

class MemoryView
{
public:
    [[nodiscard]] virtual std::uint8_t get(std::uint16_t address) const = 0;
    [[nodiscard]] virtual std::size_t size() const = 0;
};

#endif //MICRALN_MEMORYVIEW_H
