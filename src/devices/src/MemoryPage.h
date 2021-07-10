#ifndef MICRALN_MEMORYPAGE_H
#define MICRALN_MEMORYPAGE_H

#include <cstdint>
#include <span>

class MemoryPage
{
public:
    virtual ~MemoryPage();
    virtual void write(std::uint16_t address, std::uint8_t data) = 0;
    [[nodiscard]] virtual std::uint8_t read(std::uint16_t address) const = 0;
};

class ActiveMemoryPage : public MemoryPage
{
public:
    explicit ActiveMemoryPage(std::span<std::uint8_t> buffer);

    void write(std::uint16_t address, std::uint8_t data) override;
    [[nodiscard]] std::uint8_t read(std::uint16_t address) const override;

private:
    std::span<std::uint8_t> buffer;
};

class InactiveMemoryPage : public MemoryPage
{
public:
    InactiveMemoryPage() = default;

    void write(std::uint16_t address, std::uint8_t data) override;
    [[nodiscard]] std::uint8_t read(std::uint16_t address) const override;
};

#endif //MICRALN_MEMORYPAGE_H
