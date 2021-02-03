#include "CPU8008.h"
#include <emulation_core/src/Edge.h>

namespace
{
    const Scheduling::counter_type BOOT_UP_TIME = 16'000;

    namespace Timings
    {
        const Scheduling::counter_type MIN_CLOCK_PERIOD = 2000;
        const Scheduling::counter_type MAX_CLOCK_PERIOD = 3000;
        const Scheduling::counter_type PULSE_WIDTH_PHASE_1 = 700;
        const Scheduling::counter_type PULSE_WIDTH_PHASE_2 = 550;
        const Scheduling::counter_type FALLING_1_TO_FALLING_2_EDGE_MIN = 900;
        const Scheduling::counter_type FALLING_1_TO_FALLING_2_EDGE_MAX = 1100;
        const Scheduling::counter_type DELAY_FROM_PHASE_1_TO_2 = 200;
        const Scheduling::counter_type DELAY_FROM_PHASE_2_TO_1 = 400;
    } // namespace Timings

} // namespace

bool operator<(const CPU8008::NextEventType& a, const CPU8008::NextEventType& b)
{
    return std::get<0>(a) > std::get<0>(b);
}

CPU8008::CPU8008() : next_state(CpuState::STOPPED), is_first_phase_cycle(true)
{
    output_pins.state = CpuState::STOPPED;
    output_pins.sync = ::State::LOW;
}

void CPU8008::step()
{
    assert((input_pins.vdd == ::State::HIGH) && "CPU without power should not be scheduled");

    if (next_events.empty())
    {
        set_next_activation_time(Scheduling::unscheduled());
        return;
    }

    auto [time, event, param] = next_events.top();
    next_events.pop();

    switch (event)
    {
        case SYNC: {
            auto sync_edge =
                    param ? Edge{Edge::Front::RISING, time} : Edge{Edge::Front::FALLING, time};
            output_pins.sync = sync_edge.apply();
            sync_callback(sync_edge);
            // TODO : Constraint :  tSD max (.70) after ø21 FALL
            break;
        }
        case STATE: {
            // TODO: Constraint : tS2 max (1.0) after ø11 RAISE (for T1/TI1)
            // TODO: Constraint : tS1 max (1.1) after ø11 RAISE (for others)
            output_pins.state = static_cast<CpuState>(param);

            break;
        }
        case DATA_OUT:
            break;
        case DATA_INT:
            break;
    }

    if (next_events.empty())
    {
        set_next_activation_time(Scheduling::unscheduled());
    }
    else
    {
        auto& next_event = next_events.top();
        set_next_activation_time(std::get<0>(next_event));
    }
    /*
     * Events
     * SYNC (RAISE/FALL), RAISE by ø11, LOW by ø12 (+ a bit ?)

     * DATA_OUT (DATA) : Change by ø11
        *  Constraint : tDD max (1.0) after ø11 RAISE
        *  Constraint : tOH min (.10) after
     * DATA_IN // SAMPLING : After DATA OUT
     */
}

const CPU8008::OutputPins& CPU8008::get_output_pins() const { return output_pins; }
const CPU8008::DataPins& CPU8008::get_data_pins() const { return data_pins; }

void CPU8008::signal_phase_1(Edge edge)
{
    if (input_pins.vdd == ::State::LOW)
    {
        set_next_activation_time(Scheduling::unscheduled());
        return;
    }

    auto edge_time = edge.time();

    if (edge == Edge::Front::RISING)
    {
        if (is_first_phase_cycle)
        {
            switch (output_pins.state)
            {
                case CpuState::STOPPED:
                case CpuState::WAIT:
                    output_pins.state = next_state; // Shortcut
                    break;
                case CpuState::T1:
                case CpuState::T1I:
                case CpuState::T2:
                case CpuState::T3:
                case CpuState::T4:
                case CpuState::T5:
                    next_events.push(std::make_tuple(edge_time + 20, SYNC, 1));
                    break;
            }
            switch (output_pins.state)
            {
                case CpuState::STOPPED:
                case CpuState::WAIT:
                    output_pins.state = next_state; // Shortcut
                    break;
                case CpuState::T1:
                case CpuState::T1I:
                    next_events.push(
                            std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T2)));
                    break;
                case CpuState::T2:
                    next_events.push(
                            std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T3)));
                    break;
                case CpuState::T3:
                    next_events.push(
                            std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T4)));
                    break;
                case CpuState::T4:
                    next_events.push(
                            std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T5)));
                    break;
                case CpuState::T5:
                    next_events.push(
                            std::make_tuple(edge_time + 25, STATE, static_cast<int>(CpuState::T1)));

                    break;
            }
        }
        else
        {
            switch (output_pins.state)
            {
                case CpuState::STOPPED:
                case CpuState::WAIT:
                    output_pins.state = next_state; // Shortcut
                    break;
                case CpuState::T1:
                case CpuState::T1I:
                case CpuState::T2:
                case CpuState::T3:
                case CpuState::T4:
                case CpuState::T5:
                    next_events.push(std::make_tuple(edge_time + 20, SYNC, 0));
                    break;
            }
        }
        is_first_phase_cycle = !is_first_phase_cycle;
    }

    if (next_events.empty())
    {
        set_next_activation_time(edge_time);
    }
    else
    {
        auto& next_event = next_events.top();
        set_next_activation_time(std::get<0>(next_event));
    }
}

void CPU8008::signal_phase_2(Edge edge) {}

void CPU8008::signal_vdd(Edge edge)
{
    input_pins.vdd = edge.apply();
    if (input_pins.vdd == State::LOW)
    {
        set_next_activation_time(Scheduling::unscheduled());
    }
}
void CPU8008::signal_interrupt(Edge edge)
{
    input_pins.interrupt = edge.apply();
    if (input_pins.interrupt == ::State::HIGH)
    {
        // TODO: acknowledge the interruption with correct timing
        // (by timestamping the state change for example)

        if ((edge.time() - input_pins.vdd.last_change()) < BOOT_UP_TIME)
        {
            // TODO: set garbage in the CPU state. It's too early
        }

        // TODO: aknowledge of interrupt should be immediate.
        next_state = CpuState::T1I;
    }
}
void CPU8008::register_sync_trigger(std::function<void(Edge)> callback)
{
    sync_callback = std::move(callback);
}
