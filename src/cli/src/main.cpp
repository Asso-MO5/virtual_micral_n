#include <emulation_core/src/Scheduler.h>

#include <devices/src/CPU8008.h>
#include <devices/src/DoubleClock.h>
#include <devices/src/SimpleROM.h>
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

    int counting = 0;
    clock->register_phase_1_trigger([&counting, &cpu, &rom](Edge edge) {
        cpu->signal_phase_1(edge);
        if (edge == Edge::Front::RISING)
        {
            counting += 1;
            if (cpu->get_output_pins().sync == State::HIGH &&
                cpu->get_output_pins().state == CPU8008::CpuState::T3)
            {
                assert(cpu->get_data_pins().taken == false);
                rom->signal_output_enable(Edge{Edge::Front::FALLING, edge.time()});
                rom->signal_chip_select(Edge{Edge::Front::FALLING, edge.time()});
            }
        }
        else
        {
            // ROM enabled
            if (cpu->get_output_pins().sync == State::HIGH &&
                cpu->get_output_pins().state == CPU8008::CpuState::T3)
            {
                assert(cpu->get_data_pins().taken == false);
                rom->signal_chip_select(Edge{Edge::Front::RISING, edge.time()});
                rom->signal_output_enable(Edge{Edge::Front::RISING, edge.time()});
            }
        }
    });

    static uint16_t rom_address_bus;
    clock->register_phase_2_trigger([&cpu, &rom](Edge edge) {
        cpu->signal_phase_2(edge);
        if (edge == Edge::Front::RISING)
        {
            // Read data address from the CPU
            if (cpu->get_output_pins().sync == State::LOW &&
                cpu->get_output_pins().state == CPU8008::CpuState::T1)
            {
                rom_address_bus |= cpu->get_data_pins().data;
            }
            if (cpu->get_output_pins().sync == State::LOW &&
                cpu->get_output_pins().state == CPU8008::CpuState::T2)
            {
                rom_address_bus |= (cpu->get_data_pins().data & 0x3f) << 8;
                rom->set_address(rom_address_bus);
            }
        }
        else
        {
        }
    });

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
