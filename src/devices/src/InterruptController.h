#ifndef MICRALN_INTERRUPTCONTROLLER_H
#define MICRALN_INTERRUPTCONTROLLER_H

#include "Constants8008.h"

#include <functional>
#include <memory>

class CPU8008;
class Pluribus;
class OwnedSignal;
class Edge;

const size_t INTERRUPT_LEVEL_COUNT = 8;

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
    std::array<bool, INTERRUPT_LEVEL_COUNT> requested_interrupts{};
    // enabled_interrupts
    // interruption_are_masked

    void request_signals();
    void connect_values();

    void read_required_int_from_bus(OwnedSignal& signal, uint8_t level);
    [[nodiscard]] bool has_a_requested_interrupt() const;
    [[nodiscard]] uint8_t highest_level_interrupt() const;

    void cpu_state_changed(Constants8008::CpuState old_state, Constants8008::CpuState new_state,
                           unsigned long time);
};

#endif //MICRALN_INTERRUPTCONTROLLER_H
