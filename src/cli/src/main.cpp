#include <devices/src/CPU8008.h>
#include <devices/src/InterruptAtStart.h>
#include <devices/src/Pluribus.h>
#include <devices/src/ProcessorCard.h>
#include <emulator/src/Simulator.h>
#include <loguru.hpp>
#include <stdexcept>

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

    LOG_F(INFO, "Creates the simulator");
    Simulator simulator{HARD_CODED};

    //std::vector<uint8_t> rom_data{0xc0, 0x2e, 0x00, 0x36, 0x00, 0xc7, 0x44, 0x00, 0x00};

    LOG_F(INFO, "Running a bit...");

    while (simulator.get_scheduler().get_counter() < 300000)
    {
        simulator.step(16.f, STEP_ONE_CLOCK);

        auto& cpu = simulator.get_processor_card().get_cpu();
        auto& pluribus = simulator.get_pluribus();
        auto cpu_debug_data = cpu.get_debug_data();
        LOG_F(INFO,
              "8008 sync: %i, state: %s, D0-D7: %02x, (CPU PC: %04x, IR: %02x, reg.a: %02x, "
              "reg.b: %02x, A(%02x) B(%02x) C(%02x) D(%02x) E(%02x) H(%02x) L(%02x))",
              static_cast<State::Type>(cpu.output_pins.sync.get_state()),
              STATE_STRINGS[static_cast<size_t>(*cpu.output_pins.state)],
              pluribus.data_bus_d0_7.get_value(),
              cpu_debug_data.address_stack.stack[cpu_debug_data.address_stack.stack_index],
              cpu_debug_data.instruction_register, cpu_debug_data.hidden_registers.a,
              cpu_debug_data.hidden_registers.b,
              cpu_debug_data.registers[static_cast<uint8_t>(CPU8008::Register::A)],
              cpu_debug_data.registers[static_cast<uint8_t>(CPU8008::Register::B)],
              cpu_debug_data.registers[static_cast<uint8_t>(CPU8008::Register::C)],
              cpu_debug_data.registers[static_cast<uint8_t>(CPU8008::Register::D)],
              cpu_debug_data.registers[static_cast<uint8_t>(CPU8008::Register::E)],
              cpu_debug_data.registers[static_cast<uint8_t>(CPU8008::Register::H)],
              cpu_debug_data.registers[static_cast<uint8_t>(CPU8008::Register::L)]);
    }

    LOG_F(INFO, "Finished");
    LOG_F(INFO, "Clock ran for %ld nanoseconds", simulator.get_scheduler().get_counter());

    return 0;
}
