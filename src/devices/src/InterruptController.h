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

    void signal_phase_1(const Edge& edge);
    [[nodiscard]] bool has_instruction_to_inject() const;

private:
    std::shared_ptr<Pluribus> pluribus;
    std::shared_ptr<CPU8008> cpu;
    bool applying_interrupt{};

    void request_signals();
    void connect_values();
    void cpu_state_changed(Constants8008::CpuState state, Constants8008::CpuState state1,
                           unsigned long time);
};

#endif //MICRALN_INTERRUPTCONTROLLER_H
