#include <devices/src/Clock.h>
#include <emulation_core/src/Schedulable.h>
#include <emulation_core/src/Scheduler.h>

#include <loguru.hpp>

class DummySchedulable : public SchedulableImpl
{
public:
    void step() override { set_next_activation_time(get_next_activation_time() + 10); }
};

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

    LOG_F(INFO, "Creates a dummy Schedulable");
    LOG_F(INFO, "Running a bit...");

    Scheduler scheduler;
    scheduler.add(std::make_shared<DummySchedulable>());

    while (scheduler.get_counter() < 500)
    {
        scheduler.step();
    }

    LOG_F(INFO, "Finished");

    LOG_F(INFO, "Creates a Clock");
    LOG_F(INFO, "Counting some rising edges...");

    Clock clock{1'000'000_hz};

    int counting = 0;
    clock.register_trigger([&counting](Edge edge, Scheduling::counter_type) {
        if (edge == Edge::RISING)
        {
            counting += 1;
        }
    });

    while (counting < 10)
    {
        clock.step();
    }

    LOG_F(INFO, "Finished");
    LOG_F(INFO, "Clock ran for %ld nanoseconds", clock.get_next_activation_time());

    return 0;
}
