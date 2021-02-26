#include "ProcessorCard.h"

#include <utility>

#include <devices/src/CPU8008.h>
#include <devices/src/InterruptAtStart.h>
#include <devices/src/InterruptController.h>
#include <devices/src/Pluribus.h>

ProcessorCard::ProcessorCard(ProcessorCard::Config config)
    : pluribus{std::move(config.pluribus)}, cpu{std::move(config.cpu)},
      interrupt_controller{std::move(config.interrupt_controller)},
      interrupt_at_start{std::move(config.interrupt_at_start)}
{
    set_next_activation_time(Scheduling::unscheduled());

    interrupt_controller->register_interrupt_trigger(
            [this](Edge edge) { cpu->signal_interrupt(edge); });
}

const CPU8008& ProcessorCard::get_cpu() const { return *cpu; }
InterruptController& ProcessorCard::get_interrupt_controller() { return *interrupt_controller; }
void ProcessorCard::set_wait_line(Edge edge) { cpu->signal_wait(edge); }

void ProcessorCard::step() {}
