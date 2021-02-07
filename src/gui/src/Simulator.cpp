
#include "Simulator.h"
#include "ControllerWidget.h"

Simulator::Simulator()
{
    // Simulation Setup
    auto clock = std::make_shared<DoubleClock>(500'000_hz);
    cpu = std::make_shared<CPU8008>(scheduler);

    auto& captured_cpu = cpu;

    auto& captured_phase_1_recorder = phase_1_recorder;
    auto& captured_clock_1_pulse = clock_1_pulse;
    clock->register_phase_1_trigger(
            [&captured_clock_1_pulse, &captured_cpu, &captured_phase_1_recorder](Edge edge) {
                captured_clock_1_pulse += (edge == Edge::Front::RISING ? 1 : 0);

                captured_phase_1_recorder.add(edge);
                captured_cpu->signal_phase_1(edge);
            });

    auto& captured_phase_2_recorder = phase_2_recorder;
    auto& captured_clock_2_pulse = clock_2_pulse;
    clock->register_phase_2_trigger(
            [&captured_clock_2_pulse, &captured_phase_2_recorder, &captured_cpu](Edge edge) {
                captured_clock_2_pulse += (edge == Edge::Front::RISING ? 1 : 0);

                captured_phase_2_recorder.add(edge);
                captured_cpu->signal_phase_2(edge);
            });

    auto& captured_sync_recorder = sync_recorder;
    cpu->register_sync_trigger(
            [&captured_sync_recorder](Edge edge) { captured_sync_recorder.add(edge); });

    // Starts the CPU (normally should wait some cycle before triggering the interrupt)
    cpu->signal_vdd(Edge::Front::RISING);
    cpu->signal_interrupt(Edge::Front::RISING);

    scheduler.add(cpu);
    scheduler.add(clock);
}

void Simulator::step(float average_frame_time, ControllerWidget::State controller_state)
{
    if (controller_state != ControllerWidget::PAUSED)
    {
        if (controller_state == ControllerWidget::RUNNING ||
            controller_state == ControllerWidget::STEP_ONE_FRAME)
        {
            auto start_point = scheduler.get_counter();
            uint64_t end_point = start_point + (static_cast<uint64_t>(average_frame_time * 1000.f));

            while (scheduler.get_counter() < end_point)
            {
                scheduler.step();
            }
        }
        else if (controller_state == ControllerWidget::STEP_ONE_STATE)
        {
            auto initial_state = cpu->get_output_pins().state;

            while (cpu->get_output_pins().state == initial_state)
            {
                scheduler.step();
            }
        }
        else if (controller_state == ControllerWidget::STEP_ONE_CLOCK)
        {
            auto initial_clock_1 = clock_1_pulse;
            auto initial_clock_2 = clock_2_pulse;

            while (initial_clock_1 == clock_1_pulse && initial_clock_2 == clock_2_pulse)
            {
                scheduler.step();
            }
        }
    }
}

const Scheduler& Simulator::get_scheduler() const { return scheduler; }
const CPU8008& Simulator::get_cpu() const { return *cpu; }
