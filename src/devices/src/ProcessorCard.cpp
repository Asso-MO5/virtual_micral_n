#include "ProcessorCard.h"

#include <utility>

#include <devices/src/CPU8008.h>
#include <devices/src/InterruptAtStart.h>
#include <devices/src/InterruptController.h>
#include <devices/src/Pluribus.h>

ProcessorCard::ProcessorCard(ProcessorCard::Config config)
    : pluribus{std::move(config.pluribus)}, cpu{std::move(config.cpu)},
      interrupt_controller{std::move(config.interrupt_controller)},
      interrupt_at_start{std::move(config.interrupt_at_start)},
      scheduler(config.scheduler)
{
    set_next_activation_time(Scheduling::unscheduled());

    combined_ready.request(this);

    pluribus->stop.request(this);
    pluribus->wait.request(this);
    pluribus->t2.request(this);
    pluribus->t3.request(this);
    pluribus->t3prime.request(this);
    pluribus->cc0.request(this);
    pluribus->cc1.request(this);
    pluribus->address_bus_s0_s13.request(this);
    pluribus->sync.request(this);

    interrupt_controller->register_interrupt_trigger(
            [this](Edge edge) { cpu->signal_interrupt(edge); });

    pluribus->vdd.subscribe([this](Edge edge) {
        cpu->signal_vdd(edge);
        interrupt_at_start->signal_vdd(edge);

        if (edge == Edge::Front::RISING)
        {
            pluribus->ready.request(this);
            pluribus->ready.set(State::HIGH, 0, this);
            //pluribus->ready.release(this);
        }
    });

    cpu->register_state_change([this](Constants8008::CpuState old_state,
                                      Constants8008::CpuState new_state,
                                      Scheduling::counter_type time) {
        cpu_state_changed(old_state, new_state, time);
    });

    cpu->register_sync_trigger([this](Edge edge) { cpu_sync_changed(edge); });

    combined_ready.subscribe([this](Edge edge) { cpu->signal_ready(edge); });

    pluribus->ready_console.subscribe([this](Edge edge) { on_ready_change(edge); });
    pluribus->ready.subscribe([this](Edge edge) { on_ready_change(edge); });
}

const CPU8008& ProcessorCard::get_cpu() const { return *cpu; }
InterruptController& ProcessorCard::get_interrupt_controller() { return *interrupt_controller; }
void ProcessorCard::set_wait_line(Edge edge) { cpu->signal_ready(edge); }

void ProcessorCard::cpu_state_changed(Constants8008::CpuState old_state,
                                      Constants8008::CpuState state, Scheduling::counter_type time)
{
    switch (state)
    {
        case Constants8008::CpuState::STOPPED:
            pluribus->stop.set(State{State::HIGH}, time, this);
            pluribus->t2.set(State{State::LOW}, time, this);
            pluribus->t3.set(State{State::LOW}, time, this);
            pluribus->t3prime.set(State{State::LOW}, time, this);
            break;
        case Constants8008::CpuState::T1I:
            pluribus->stop.set(State{State::LOW}, time, this);
            pluribus->t2.set(State{State::LOW}, time, this);
            pluribus->t3.set(State{State::LOW}, time, this);
            pluribus->t3prime.set(State{State::LOW}, time, this);
            break;
        case Constants8008::CpuState::T1:
            pluribus->t2.set(State{State::LOW}, time, this);
            pluribus->t3.set(State{State::LOW}, time, this);
            pluribus->t3prime.set(State{State::LOW}, time, this);
            break;
        case Constants8008::CpuState::T2:
            pluribus->t2.set(State{State::HIGH}, time, this);
            pluribus->t3.set(State{State::LOW}, time, this);
            pluribus->t3prime.set(State{State::LOW}, time, this);
            break;
        case Constants8008::CpuState::WAIT:
            pluribus->wait.set(State{State::HIGH}, time, this);
            pluribus->t2.set(State{State::LOW}, time, this);
            pluribus->t3.set(State{State::LOW}, time, this);
            break;
        case Constants8008::CpuState::T3:
            pluribus->wait.set(State{State::LOW}, time, this);
            pluribus->t2.set(State{State::LOW}, time, this);
            pluribus->t3.set(State{State::HIGH}, time, this);
            break;
        case Constants8008::CpuState::T4:
            pluribus->t2.set(State{State::LOW}, time, this);
            pluribus->t3.set(State{State::LOW}, time, this);
            pluribus->t3prime.set(State{State::LOW}, time, this);
            break;
        case Constants8008::CpuState::T5:
            pluribus->t2.set(State{State::HIGH}, time, this);
            pluribus->t3.set(State{State::LOW}, time, this);
            pluribus->t3prime.set(State{State::LOW}, time, this);
            break;
    }

    if (old_state == Constants8008::CpuState::T2)
    {
        // T'3 is generated from the end of T2, not depending on T3.
        emit_t3prime_on_next_step = true;
        set_next_activation_time(time + 500);
        scheduler.change_schedule(get_id());
    }
}

void ProcessorCard::cpu_sync_changed(Edge edge)
{
    if (edge == Edge{Edge::Front::FALLING})
    {
        auto cpu_state = *cpu->get_output_pins().state;
        if (cpu_state == Constants8008::CpuState::T1)
        {
            latched_address &= 0xff00;
            latched_address |= cpu->get_data_pins().read();
        }
        else if (cpu_state == Constants8008::CpuState::T2)
        {
            auto read_value = cpu->get_data_pins().read();

            latched_address &= 0x00ff;
            latched_address |= (read_value & 0x3f) << 8;

            latched_cycle_control =
                    static_cast<Constants8008::CycleControl>(read_value & 0b11000000);

            auto cc0 = (read_value & 0b10000000) >> 7;
            auto cc1 = (read_value & 0b01000000) >> 6;

            auto time = edge.time();

            pluribus->address_bus_s0_s13.set(latched_address, time, this);
            pluribus->cc0.set(cc0, time, this);
            pluribus->cc1.set(cc1, time, this);
        }
    }
    pluribus->sync.apply(edge, this);
}

void ProcessorCard::on_ready_change(Edge edge)
{
    combined_ready.set((*pluribus->ready_console) && (*pluribus->ready), edge.time(), this);
}

void ProcessorCard::step()
{

    assert(emit_t3prime_on_next_step &&
           "Currently, step() in the ProcessorCard only purpose is to emit t3prime.");
    emit_t3prime_on_next_step = false;

    auto time = get_next_activation_time();
    pluribus->t3prime.set(State{State::HIGH}, time, this);

    set_next_activation_time(Scheduling::unscheduled());
}
