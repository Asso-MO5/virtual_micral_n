#include <devices/src/CPU8008.h>
#include <devices/src/ControlBus.h>
#include <devices/src/DoubleClock.h>
#include <devices/src/InterruptAtStart.h>
#include <devices/src/SimpleROM.h>
#include <devices/src/SimpleRAM.h>
#include <emulation_core/src/Scheduler.h>

#include <emulation_core/src/DataBus.h>
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

    LOG_F(INFO, "Creates the scheduler");
    Scheduler scheduler;

    LOG_F(INFO, "Creates the ROM");
    std::vector<uint8_t> rom_data{0xc0, 0x2e, 0x00, 0x36, 0x00, 0xc7, 0x44, 0x00, 0x00};
    auto rom = std::make_shared<SimpleROM>(rom_data);
    auto ram = std::make_shared<SimpleRAM>(1);; // Not used here, but the ControlBus needs one.

    LOG_F(INFO, "Creates the 8008");
    auto cpu = std::make_shared<CPU8008>(scheduler);

    auto data_bus = std::make_shared<DataBus>();
    cpu->connect_data_bus(data_bus);
    rom->connect_data_bus(data_bus);

    LOG_F(INFO, "Creates the Clock");
    auto clock = std::make_shared<DoubleClock>(500'000_hz);

    ControlBus control_bus{cpu, rom, ram};
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

        auto cpu_debug_data = cpu->get_debug_data();
        LOG_F(INFO,
              "8008 sync: %i, state: %s, data bus: %02x, (CPU PC: %04x, IR: %02x, reg.a: %02x, "
              "reg.b: %02x, A(%02x) B(%02x) C(%02x) D(%02x) E(%02x) H(%02x) L(%02x))",
              static_cast<State::Type>(cpu->get_output_pins().sync),
              STATE_STRINGS[static_cast<size_t>(cpu->get_output_pins().state)], data_bus->read(),
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
    LOG_F(INFO, "Clock ran for %ld nanoseconds", clock->get_next_activation_time());

    return 0;
}
