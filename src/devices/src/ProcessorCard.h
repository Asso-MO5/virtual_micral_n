#ifndef MICRALN_PROCESSORCARD_H
#define MICRALN_PROCESSORCARD_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>
#include <memory>

class Pluribus;
class CPU8008;
class InterruptController;
class InterruptAtStart;
class DoubleClock;

class ProcessorCard : public SchedulableImpl
{
public:
    struct Config
    {
        std::shared_ptr<Pluribus> pluribus;
        std::shared_ptr<DoubleClock> clock;
        std::shared_ptr<CPU8008> cpu;
        std::shared_ptr<InterruptController> interrupt_controller;
        std::shared_ptr<InterruptAtStart> interrupt_at_start;
    };

    explicit ProcessorCard(Config config);
    ~ProcessorCard() override = default;

    void step() override;

    [[nodiscard]] const CPU8008& get_cpu() const;
    [[nodiscard]] InterruptController& get_interrupt_controller();
    void set_wait_line(Edge edge);

private:
    std::shared_ptr<Pluribus> pluribus;
    std::shared_ptr<CPU8008> cpu{};
    std::shared_ptr<InterruptController> interrupt_controller;
    std::shared_ptr<InterruptAtStart> interrupt_at_start;

    void cpu_state_changed();
};

#endif //MICRALN_PROCESSORCARD_H
