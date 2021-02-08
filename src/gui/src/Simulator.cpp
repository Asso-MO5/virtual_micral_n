
#include "Simulator.h"
#include "ControllerWidget.h"

#include <devices/src/CPU8008.h>
#include <devices/src/ControlBus.h>
#include <devices/src/InterruptAtStart.h>
#include <devices/src/SimpleROM.h>

Simulator::Simulator()
{
    std::vector<uint8_t> rom_data{0xc0, 0x2e, 0x00, 0x36, 0x00, 0xc7, 0x44, 0x00, 0x00};

    // Simulation Setup
    auto clock = std::make_shared<DoubleClock>(500'000_hz);
    cpu = std::make_shared<CPU8008>(scheduler);
    rom = std::make_shared<SimpleROM>(rom_data);
    data_bus = std::make_shared<DataBus>();
    interrupt_at_start = std::make_shared<InterruptAtStart>(cpu);
    control_bus = std::make_shared<ControlBus>(cpu, rom);

    cpu->connect_data_bus(data_bus);
    rom->connect_data_bus(data_bus);

    clock->register_phase_1_trigger([this](Edge edge) {
        clock_1_pulse += (edge == Edge::Front::RISING ? 1 : 0);

        phase_1_recorder.add(edge);
        cpu->signal_phase_1(edge);
        interrupt_at_start->signal_phase_1(edge);
        control_bus->signal_phase_1(edge);
    });

    clock->register_phase_2_trigger([this](Edge edge) {
        clock_2_pulse += (edge == Edge::Front::RISING ? 1 : 0);
        phase_2_recorder.add(edge);

        cpu->signal_phase_2(edge);
        control_bus->signal_phase_2(edge);
    });

    auto& captured_sync_recorder = sync_recorder;
    cpu->register_sync_trigger(
            [&captured_sync_recorder](Edge edge) { captured_sync_recorder.add(edge); });

    // Starts the CPU (normally should wait some cycle before triggering the interrupt)
    cpu->signal_vdd(Edge::Front::RISING);
    interrupt_at_start->signal_vdd(Edge::Front::RISING);

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
const DataBus& Simulator::get_data_bus() const { return *data_bus; }
