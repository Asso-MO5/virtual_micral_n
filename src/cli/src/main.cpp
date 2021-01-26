#include <emulation_core/src/Schedulable.h>
#include <emulation_core/src/Scheduler.h>

#include <loguru.hpp>

class DummySchedulable : public Schedulable
{
public:
    void step() override { next_actions += 10; }
    [[nodiscard]] Scheduling::counter_type get_next_activation_time() const override
    {
        return next_actions;
    }

private:
    Scheduling::counter_type next_actions = 0;
};

int main(int argc, char** argv)
{
#ifdef NDEBUG
    loguru::g_stderr_verbosity = loguru::Verbosity_WARNING;
#else
    loguru::g_stderr_verbosity = loguru::Verbosity_INFO;
#endif
    loguru::set_fatal_handler([](const loguru::Message& message){
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

    return 0;
}
