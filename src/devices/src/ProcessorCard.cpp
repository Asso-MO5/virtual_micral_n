#include "ProcessorCard.h"

#include "AutomaticStart.h"
#include "CPU8008.h"
#include "Clock.h"
#include "DoubleClock.h"
#include "GeneralAddressRegister.h"
#include "InterruptCircuit.h"
#include "InterruptController.h"
#include "Pluribus.h"

#include <utility>

ProcessorCard::ProcessorCard(ProcessorCard::Config config)
    : pluribus{std::move(config.pluribus)}, change_schedule{config.change_schedule}
{
    set_next_activation_time(Scheduling::unscheduled());

    clock = std::make_shared<DoubleClock>(500'000_hz);
    cpu = std::make_shared<CPU8008>(change_schedule);
    bus_address_decoder = std::make_shared<GeneralAddressRegister>(cpu, pluribus);
    interrupt_controller =
            std::make_shared<InterruptController>(pluribus, cpu, bus_address_decoder);
    automatic_startup = std::make_shared<AutomaticStart>(cpu);
    real_time_clock = std::make_shared<Clock>(100_hz); // Default factory configuration.

    connect_to_pluribus();
    connect_to_cpu();
    connect_to_clock();
    connect_to_rtc();

    cpu->input_pins.ready.request(this);
    combined_ready.request(this);
    combined_ready.subscribe([this](Edge edge) { cpu->input_pins.ready.apply(edge, this); });
}

ProcessorCard::~ProcessorCard() = default;

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
    pluribus->sync.request(this);
    pluribus->data_bus_d0_7.request(this, 0);

    pluribus->ready_console.subscribe([this](Edge edge) { on_ready(edge); });
    pluribus->ready.subscribe([this](Edge edge) { on_ready(edge); });
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

    cpu->data_pins.subscribe([this](uint8_t, uint8_t new_value, Scheduling::counter_type time) {
        pluribus->data_bus_d0_7.set(new_value, time, this);
    });
}

void ProcessorCard::connect_to_rtc()
{
    // The connection between RTC and bi7 is permanent for a given setting.
    // It's a soldered jumper on the Processor Board.

    bi7_interrupt_controller = std::make_unique<InterruptCircuit>(pluribus->bi7, pluribus->aint7);

    real_time_clock->phase.subscribe([this](Edge edge) {
        if (is_low(pluribus->bi7) && is_rising(edge) && automatic_startup->is_ready())
        {
            bi7_interrupt_controller->trigger(edge.time());
        }
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
        change_schedule(get_id());
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

void ProcessorCard::cpu_sync_changed(Edge edge) { pluribus->sync.apply(edge, this); }

void ProcessorCard::on_ready(Edge edge)
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
        if (is_high(pluribus->t3))
        {
            auto cycle_control = bus_address_decoder->get_latched_cycle_control();
            if (cycle_control == Constants8008::CycleControl::PCI ||
                cycle_control == Constants8008::CycleControl::PCR ||
                cycle_control == Constants8008::CycleControl::PCC)
            {
                auto time = edge.time();
                if (t1i_cycle && interrupt_controller->has_instruction_to_inject())
                {
                    auto instruction_to_inject = interrupt_controller->get_instruction_to_inject();
                    cpu->data_pins.request(this, time);
                    cpu->data_pins.set(instruction_to_inject, time, this);
                    cpu->data_pins.release(this, time);

                    interrupt_controller->reset_lowest_interrupt();
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
            debug_info.watchdog_on = interrupt_controller->is_watchdog_on();
        }
    });
}

const ProcessorCard::DebugData& ProcessorCard::get_debug_data() const { return debug_info; }
