#include "AddressStack.h"

AddressStack::AddressStack() { clear_stack(); }
AddressStack::AddressStack(uint16_t pc)
{
    clear_stack();
    stack[stack_index] = pc;
}

void AddressStack::clear_stack()
{
    std::fill(begin(stack), end(stack), 0);
    stack_index = 0;
}

uint16_t AddressStack::get_pc() const { return stack[stack_index]; }

uint16_t AddressStack::get_low_pc_and_inc()
{
    // Note: the 8008 normally waits T2 to increment the 6 high bits part of the PC
    // It's done directly in the simulation, as T1 and T2 are not supposed to be interrupted.
    auto& pc = stack[stack_index];
    emitted_pc = pc;
    pc = (pc + 1) & 0x3fff;
    return emitted_pc & 0xff;
}

uint16_t AddressStack::get_low_pc_no_inc()
{
    emitted_pc = stack[stack_index];
    return emitted_pc & 0xff;
}

uint16_t AddressStack::get_high_pc() const { return (emitted_pc & 0x3f00) >> 8; }

void AddressStack::push()
{
    stack_index = (stack_index + 1) % stack.size();
}

void AddressStack::pop()
{
    stack_index = (stack_index - 1);
    if (stack_index < 0)
    {
        stack_index = stack.size() - 1;
    }
}

void AddressStack::set_low_pc(uint8_t value) {
    stack[stack_index] &= 0xff00;
    stack[stack_index] |= value;
}

void AddressStack::set_high_pc(uint8_t  value) {
    stack[stack_index] &= 0x00ff;
    stack[stack_index] |= value << 8;
}

