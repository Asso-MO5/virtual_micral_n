#ifndef MICRALN_INTERRUPTCONTROLLER_H
#define MICRALN_INTERRUPTCONTROLLER_H

#include "Constants8008.h"

#include <emulation_core/src/Edge.h>
#include <functional>

class InterruptController
{
public:
    // Needs the CPU State by Reference, to detect T1I
    explicit InterruptController(const Constants8008::CpuState* cpu_state);

    void wants_interrupt(const Edge& edge);
    void signal_phase_1(const Edge& edge);
    void register_interrupt_trigger(std::function<void(Edge)> callback);

private:
    const Constants8008::CpuState* cpu_state;
    std::function<void(Edge)> interrupt_callback;

    bool interrupt_is_scheduled{};
    bool applying_interrupt{};
};

#endif //MICRALN_INTERRUPTCONTROLLER_H
