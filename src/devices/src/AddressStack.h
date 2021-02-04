#ifndef MICRALN_ADDRESSSTACK_H
#define MICRALN_ADDRESSSTACK_H

#include <cstdint>
#include <array>

const std::size_t ADDRESS_STACK_LEVELS = 8;

class AddressStack
{
public:
    AddressStack();
    explicit AddressStack(uint16_t pc);
    [[nodiscard]] uint16_t get_pc() const;
    [[nodiscard]] uint16_t get_low_pc_and_inc();
    [[nodiscard]] uint16_t get_low_pc_no_inc();
    [[nodiscard]] uint16_t get_high_pc() const;

    void push(uint16_t address);
    void pop();

private:
    std::array<uint16_t, ADDRESS_STACK_LEVELS> stack{};
    std::size_t stack_index{};
    uint16_t emitted_pc{};

    void clear_stack();
};

#endif //MICRALN_ADDRESSSTACK_H
