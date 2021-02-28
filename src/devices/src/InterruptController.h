#ifndef MICRALN_INTERRUPTCONTROLLER_H
#define MICRALN_INTERRUPTCONTROLLER_H

#include "Constants8008.h"

#include <emulation_core/src/Edge.h>
#include <functional>

class InterruptController
{
public:
    explicit InterruptController();

    void wants_interrupt(const Edge& edge);
    void signal_phase_1(const Edge& edge);
    void register_interrupt_trigger(std::function<void(Edge)> callback);

    void on_state_value_change(Constants8008::CpuState old_value, Constants8008::CpuState new_value,
                               Scheduling::counter_type time);

private:
    Constants8008::CpuState latest_cpu_state{};
    std::function<void(Edge)> interrupt_callback;

    bool interrupt_is_scheduled{};
    bool applying_interrupt{};
};

#endif //MICRALN_INTERRUPTCONTROLLER_H
