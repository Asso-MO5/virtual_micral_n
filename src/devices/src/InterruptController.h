#ifndef MICRALN_INTERRUPTCONTROLLER_H
#define MICRALN_INTERRUPTCONTROLLER_H

#include "Constants8008.h"

#include <emulation_core/src/Edge.h>
#include <functional>
#include <memory>

class CPU8008;
class Pluribus;

class InterruptController
{
public:
    explicit InterruptController(std::shared_ptr<Pluribus> pluribus, std::shared_ptr<CPU8008> cpu);

    void on_phase_1(const Edge& edge);
    [[nodiscard]] bool has_instruction_to_inject() const;
    [[nodiscard]] uint8_t get_instruction_to_inject() const;
    void reset_interrupt(uint8_t interrupt_level);

private:
    std::shared_ptr<Pluribus> pluribus;
    std::shared_ptr<CPU8008> cpu;
    bool applying_interrupt{};
    bool pending_int_level_0{}; // TODO: change this to an array of pending interrupt levels

    void request_signals();
    void connect_values();
    void cpu_state_changed(Constants8008::CpuState state, Constants8008::CpuState state1,
                           unsigned long time);
};

#endif //MICRALN_INTERRUPTCONTROLLER_H
