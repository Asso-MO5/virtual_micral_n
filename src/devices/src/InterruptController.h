#ifndef MICRALN_INTERRUPTCONTROLLER_H
#define MICRALN_INTERRUPTCONTROLLER_H

#include "Constants8008.h"

#include <emulation_core/src/Edge.h>
#include <functional>
#include <memory>

class CPU8008;

class InterruptController
{
public:
    explicit InterruptController(std::shared_ptr<CPU8008> cpu);

    void signal_phase_1(const Edge& edge);
    void register_interrupt_trigger(std::function<void(Edge)> callback);
    void on_init_changed(const Edge& edge);

private:
    std::shared_ptr<CPU8008> cpu;
    std::function<void(Edge)> interrupt_callback;

    bool interrupt_is_scheduled{};
    bool applying_interrupt{};
};

#endif //MICRALN_INTERRUPTCONTROLLER_H
