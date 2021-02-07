#include <devices/src/CPU8008.h>
#include <devices/src/ControlBus.h>
#include <devices/src/DoubleClock.h>
#include <devices/src/InterruptAtStart.h>
#include <devices/src/SimpleROM.h>
#include <emulation_core/src/Scheduler.h>

#include <loguru.hpp>

const char* STATE_STRINGS[] = {"WAIT", "T3", "T1", "STOPPED", "T2", "T5", "T1I", "T4"};

int main(int argc, char** argv)
{
#ifdef NDEBUG
    loguru::g_stderr_verbosity = loguru::Verbosity_WARNING;
#else
    loguru::g_stderr_verbosity = loguru::Verbosity_INFO;
#endif
    loguru::set_fatal_handler([](const loguru::Message& message) {
        throw std::runtime_error(std::string(message.prefix) + message.message);
    });

    loguru::init(argc, argv);

    LOG_F(INFO, "Creates the scheduler");
    Scheduler scheduler;

    LOG_F(INFO, "Creates the ROM");
    std::vector<uint8_t> rom_data{0xc0, 0x2e, 0x00, 0x36, 0x00, 0xc7, 0x44, 0x00, 0x00};
    auto rom = std::make_shared<SimpleROM>(rom_data);

    LOG_F(INFO, "Creates the 8008");
    auto cpu = std::make_shared<CPU8008>(scheduler);

    LOG_F(INFO, "Creates the Clock");
    auto clock = std::make_shared<DoubleClock>(500'000_hz);

    ControlBus control_bus{cpu, rom};
    InterruptAtStart interrupt_at_start{cpu};

    clock->register_phase_1_trigger([&interrupt_at_start, &cpu, &control_bus](Edge edge) {
        interrupt_at_start.signal_phase_1(edge);
        cpu->signal_phase_1(edge);
        control_bus.signal_phase_1(edge);
    });

    clock->register_phase_2_trigger([&cpu, &control_bus](Edge edge) {
        cpu->signal_phase_2(edge);
        control_bus.signal_phase_2(edge);
    });

    // What I would like to write:
    // connect(&DoubleClock::register_phase_1_trigger, &CPU8008::signal_phase_1);

    LOG_F(INFO, "Adds devices to the scheduler");
    scheduler.add(clock);
    scheduler.add(cpu);

    LOG_F(INFO, "Starts the CPU");
    cpu->signal_vdd(Edge::Front::RISING);
    interrupt_at_start.signal_vdd(Edge::Front::RISING);

    LOG_F(INFO, "Running a bit...");

    while (clock->get_next_activation_time() < 300000)
    {
        scheduler.step();
        scheduler.step();

        LOG_F(INFO, "8008 sync: %i, state: %s, data bus: %02x, rom output: %01i/%02x",
              static_cast<State::Type>(cpu->get_output_pins().sync),
              STATE_STRINGS[static_cast<size_t>(cpu->get_output_pins().state)],
              cpu->get_data_pins().data, rom->get_data_pins().taken, rom->get_data_pins().data);
    }

    LOG_F(INFO, "Finished");
    LOG_F(INFO, "Clock ran for %ld nanoseconds", clock->get_next_activation_time());

    return 0;
}
