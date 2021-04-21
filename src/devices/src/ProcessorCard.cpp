#include "ProcessorCard.h"
#include "Clock.h"

#include <devices/src/AutomaticStart.h>
#include <devices/src/CPU8008.h>
#include <devices/src/DoubleClock.h>
#include <devices/src/InterruptController.h>
#include <devices/src/Pluribus.h>
#include <emulation_core/src/DataBus.h>
#include <utility>

ProcessorCard::ProcessorCard(ProcessorCard::Config config)
    : scheduler{config.scheduler}, pluribus{std::move(config.pluribus)}
{
    set_next_activation_time(Scheduling::unscheduled());

    clock = std::make_shared<DoubleClock>(500'000_hz);
    cpu = std::make_shared<CPU8008>(scheduler);
    interrupt_controller = std::make_shared<InterruptController>(pluribus, cpu);
    automatic_startup = std::make_shared<AutomaticStart>(cpu);
    real_time_clock = std::make_shared<Clock>(100_hz); // Default factory configuration.

    connect_to_pluribus();
    connect_to_cpu();
    connect_to_clock();

    cpu->input_pins.ready.request(this);
    combined_ready.request(this);
    combined_ready.subscribe([this](Edge edge) { cpu->input_pins.ready.apply(edge, this); });
}

void ProcessorCard::connect_to_clock()
{
    clock->phase_1.subscribe([this](Edge edge) {
        cpu->signal_phase_1(edge);
        automatic_startup->on_phase_1(edge);
        interrupt_controller->on_phase_1(edge);
    });

    clock->phase_2.subscribe([this](Edge edge) {
        pluribus->phase_2.apply(edge, this);
        cpu->signal_phase_2(edge);
    });
}

void ProcessorCard::connect_to_cpu()
{
    cpu->register_state_change([this](Constants8008::CpuState old_state,
                                      Constants8008::CpuState new_state,
                                      Scheduling::counter_type time) {
        cpu_state_changed(old_state, new_state, time);
    });

    cpu->register_sync_trigger([this](Edge edge) { cpu_sync_changed(edge); });
}

void ProcessorCard::connect_to_pluribus()
{
    pluribus->phase_2.request(this);
    pluribus->stop.request(this);
    pluribus->wait.request(this);
    pluribus->t2.request(this);
    pluribus->t3.request(this);
    pluribus->t3prime.request(this);
    pluribus->cc0.request(this);
    pluribus->cc1.request(this);
    pluribus->address_bus_s0_s13.request(this, 0);
    pluribus->sync.request(this);
    pluribus->data_bus_d0_7.request(this, 0);

    pluribus->ready_console.subscribe([this](Edge edge) { on_ready_change(edge); });
    pluribus->ready.subscribe([this](Edge edge) { on_ready_change(edge); });
    pluribus->phase_2.subscribe([this](Edge edge) { on_phase_2(edge); });

    cpu->input_pins.vdd.request(this);
    pluribus->vdd.subscribe([this](Edge edge) {
        cpu->input_pins.vdd.apply(edge, this);
        automatic_startup->on_vdd(edge);

        if (is_rising(edge))
        {
            pluribus->ready.request(this);
            pluribus->ready.set(State::HIGH, 0, this);
            pluribus->ready.release(this);
        }
    });

    cpu->data_pins.subscribe(
            [this](uint8_t old_value, uint8_t new_value, Scheduling::counter_type time) {
                pluribus->data_bus_d0_7.set(new_value, time, this);
            });
}

const CPU8008& ProcessorCard::get_cpu() const { return *cpu; }
DoubleClock& ProcessorCard::get_clock() { return *clock; }
Clock& ProcessorCard::get_rtc() { return *real_time_clock; }

void ProcessorCard::cpu_state_changed(Constants8008::CpuState old_state,
                                      Constants8008::CpuState state, Scheduling::counter_type time)
{
    apply_signal_on_bus(state, time);

    if (old_state == Constants8008::CpuState::T2)
    {
        // T'3 is generated from the end of T2, not depending on T3.
        emit_t3prime_on_next_step = true;
        set_next_activation_time(time + 500);
        scheduler.change_schedule(get_id());
    }
}

void ProcessorCard::apply_signal_on_bus(const Constants8008::CpuState& state, unsigned long time)
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
        case Constants8008::CpuState::T5:
            pluribus->t2.set(State{State::LOW}, time, this);
            pluribus->t3.set(State{State::LOW}, time, this);
            pluribus->t3prime.set(State{State::LOW}, time, this);
            break;
    }
}

void ProcessorCard::cpu_sync_changed(Edge edge)
{
    if (edge == Edge{Edge::Front::FALLING})
    {
        auto cpu_state = *cpu->output_pins.state;

        // TODO: Verify if the address is also sent on the BUS after an interruption
        if (cpu_state == Constants8008::CpuState::T1 || cpu_state == Constants8008::CpuState::T1I)
        {
            latched_address &= 0xff00;
            latched_address |= cpu->data_pins.get_value();
        }
        else if (cpu_state == Constants8008::CpuState::T2)
        {
            auto read_value = cpu->data_pins.get_value();

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

void ProcessorCard::on_phase_2(Edge edge)
{

    if (is_falling(edge))
    {
        if (is_high(*pluribus->t3))
        {
            if (latched_cycle_control == Constants8008::CycleControl::PCI ||
                latched_cycle_control == Constants8008::CycleControl::PCR ||
                latched_cycle_control == Constants8008::CycleControl::PCC)
            {
                auto time = edge.time();
                if (t1i_cycle && interrupt_controller->has_instruction_to_inject())
                {
                    cpu->data_pins.request(this, time);
                    cpu->data_pins.set(0x00, time, this);
                    cpu->data_pins.release(this, time);
                }
                else
                {
                    auto read_data = *pluribus->data_bus_md0_7;

                    // TODO: find better timings
                    cpu->data_pins.request(this, time);
                    cpu->data_pins.set(read_data, time, this);
                    cpu->data_pins.release(this, time);
                }
            }
        }
        else if (*cpu->output_pins.state == Constants8008::CpuState::T1I)
        {
            t1i_cycle = true;
        }
        else if (*cpu->output_pins.state == Constants8008::CpuState::T1)
        {
            t1i_cycle = false;
        }
    }
}

std::vector<std::shared_ptr<Schedulable>> ProcessorCard::get_sub_schedulables()
{
    return {clock, cpu, real_time_clock};
}

void ProcessorCard::install_debug_info()
{
    debug_info.clock_pulse = 0;
    clock->phase_1.subscribe([this](Edge edge) {
        if (is_rising(edge))
        {
            debug_info.clock_pulse += 1;
        }
    });
}

const ProcessorCard::DebugInfo& ProcessorCard::get_debug_info() const { return debug_info; }
