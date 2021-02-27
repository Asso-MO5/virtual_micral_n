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

    pluribus->vdd.subscribe([this](Edge edge) {
        cpu->signal_vdd(edge);
        interrupt_at_start->signal_vdd(edge);
    });

    cpu->register_state_change([this]() { cpu_state_changed(); });

    pluribus->stop.request(this);
    pluribus->wait.request(this);
    pluribus->t2.request(this);
    pluribus->t3.request(this);
    pluribus->t3prime.request(this);
}

const CPU8008& ProcessorCard::get_cpu() const { return *cpu; }
InterruptController& ProcessorCard::get_interrupt_controller() { return *interrupt_controller; }
void ProcessorCard::set_wait_line(Edge edge) { cpu->signal_wait(edge); }

void ProcessorCard::cpu_state_changed()
{
    const auto& state = cpu->get_output_pins().state;

    switch (state)
    {
        case Constants8008::CpuState::STOPPED:
            pluribus->stop.set(State{State::HIGH}, 0, this);   // TODO: Set timing
            pluribus->t2.set(State{State::LOW}, 0, this);      // TODO: Set timing
            pluribus->t3.set(State{State::LOW}, 0, this);      // TODO: Set timing
            pluribus->t3prime.set(State{State::LOW}, 0, this); // TODO: Set timing
            break;
        case Constants8008::CpuState::T1I:
            pluribus->stop.set(State{State::LOW}, 0, this);    // TODO: Set timing
            pluribus->t2.set(State{State::LOW}, 0, this);      // TODO: Set timing
            pluribus->t3.set(State{State::LOW}, 0, this);      // TODO: Set timing
            pluribus->t3prime.set(State{State::LOW}, 0, this); // TODO: Set timing
            break;
        case Constants8008::CpuState::T1:
            pluribus->t2.set(State{State::LOW}, 0, this);      // TODO: Set timing
            pluribus->t3.set(State{State::LOW}, 0, this);      // TODO: Set timing
            pluribus->t3prime.set(State{State::LOW}, 0, this); // TODO: Set timing
            break;
        case Constants8008::CpuState::T2:
            pluribus->t2.set(State{State::HIGH}, 0, this);     // TODO: Set timing
            pluribus->t3.set(State{State::LOW}, 0, this);      // TODO: Set timing
            pluribus->t3prime.set(State{State::LOW}, 0, this); // TODO: Set timing
            break;
        case Constants8008::CpuState::WAIT:
            pluribus->wait.set(State{State::HIGH}, 0, this);   // TODO: Set timing
            pluribus->t2.set(State{State::LOW}, 0, this);      // TODO: Set timing
            pluribus->t3.set(State{State::LOW}, 0, this);      // TODO: Set timing
            pluribus->t3prime.set(State{State::LOW}, 0, this); // TODO: Set timing
            break;
        case Constants8008::CpuState::T3:
            pluribus->wait.set(State{State::LOW}, 0, this);    // TODO: Set timing
            pluribus->t2.set(State{State::LOW}, 0, this);      // TODO: Set timing
            pluribus->t3.set(State{State::HIGH}, 0, this);     // TODO: Set timing
            pluribus->t3prime.set(State{State::LOW}, 0, this); // TODO: Set timing
            break;
        case Constants8008::CpuState::T4:
            pluribus->t2.set(State{State::LOW}, 0, this);      // TODO: Set timing
            pluribus->t3.set(State{State::LOW}, 0, this);      // TODO: Set timing
            pluribus->t3prime.set(State{State::LOW}, 0, this); // TODO: Set timing
            break;
        case Constants8008::CpuState::T5:
            pluribus->t2.set(State{State::HIGH}, 0, this);     // TODO: Set timing
            pluribus->t3.set(State{State::LOW}, 0, this);      // TODO: Set timing
            pluribus->t3prime.set(State{State::LOW}, 0, this); // TODO: Set timing
            break;
    }
}

void ProcessorCard::step() {}
