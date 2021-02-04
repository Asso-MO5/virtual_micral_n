#include <emulation_core/src/Schedulable.h>
#include <emulation_core/src/Scheduler.h>

#include <devices/src/CPU8008.h>
#include <devices/src/DoubleClock.h>
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

    LOG_F(INFO, "Creates the 8008");
    auto cpu = std::make_shared<CPU8008>(scheduler);

    LOG_F(INFO, "Creates the Clock");
    auto clock = std::make_shared<DoubleClock>(500'000_hz);

    int counting = 0;
    clock->register_phase_1_trigger([&counting, &cpu](Edge edge) {
        if (edge == Edge::Front::RISING)
        {
            counting += 1;
        }
        cpu->signal_phase_1(edge);
    });
    clock->register_phase_2_trigger([&cpu](Edge edge) { cpu->signal_phase_2(edge); });

    LOG_F(INFO, "Adds devices to the scheduler");
    scheduler.add(clock);
    scheduler.add(cpu);

    LOG_F(INFO, "Starts the CPU");
    cpu->signal_vdd(Edge::Front::RISING);

    LOG_F(INFO, "Running a bit...");

    while (clock->get_next_activation_time() < 300000)
    {
        if (counting == 20)
        {
            cpu->signal_interrupt({Edge::Front::RISING, clock->get_next_activation_time()});
        }
        if (counting == 21)
        {
            cpu->signal_interrupt(Edge{Edge::Front::FALLING, clock->get_next_activation_time()});
        }

        scheduler.step();

        LOG_F(INFO, "8008 sync: %i, state: %s",
              static_cast<State::Type>(cpu->get_output_pins().sync),
              STATE_STRINGS[static_cast<size_t>(cpu->get_output_pins().state)]);
    }

    LOG_F(INFO, "Finished");
    LOG_F(INFO, "Clock ran for %ld nanoseconds", clock->get_next_activation_time());

    return 0;
}
