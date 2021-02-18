
#include "Simulator.h"
#include "ControllerWidget.h"

#include <devices/src/CPU8008.h>
#include <devices/src/ControlBus.h>
#include <devices/src/InterruptAtStart.h>
#include <devices/src/SimpleROM.h>

#include <fstream>
#include <utility>

namespace
{
    const uint16_t RAM_SIZE = 2048;
    const uint16_t RAM_START = 0x1000;
} // namespace

class ReadRomData
{
public:
    explicit ReadRomData(const char* file_path)
    {
        std::fstream file;
        file.open(file_path, std::ios::in | std::ios::binary);
        if (file)
        {
            file.seekg(0, std::ios::end);
            auto file_size = file.tellg();
            file.seekg(0, std::ios::beg);

            data.resize(file_size);
            file.read(reinterpret_cast<char*>(&data[0]), file_size);
        }
        else
        {
            throw std::runtime_error("Cannot open ROM file");
        }
    }

    std::vector<uint8_t> data;
};

Simulator::Simulator()
{
    ReadRomData rom_data_file("data/8008-hello-world.bin");
    auto& rom_data = rom_data_file.data;

    //    std::vector<uint8_t> rom_data{0xc0, 0x2e, 0xff, 0x2e, 0x00, 0x36, 0xc0,
    //                                  0x36, 0x00, 0xc7, 0x44, 0x00, 0x00};

    // Simulation Setup
    auto clock = std::make_shared<DoubleClock>(500'000_hz);
    cpu = std::make_shared<CPU8008>(scheduler);
    rom = std::make_shared<SimpleROM>(rom_data);
    ram = std::make_shared<SimpleRAM>(RAM_SIZE);

    data_bus = std::make_shared<DataBus>();
    interrupt_at_start = std::make_shared<InterruptAtStart>(cpu);
    control_bus = std::make_shared<ControlBus>(cpu, rom, ram);

    cpu->connect_data_bus(data_bus);
    rom->connect_data_bus(data_bus);
    ram->connect_data_bus(data_bus);

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

    cpu->register_sync_trigger([this](Edge edge) {
        sync_recorder.add(edge);
        control_bus->signal_sync(edge);
    });

    // Starts the CPU (normally should wait some cycle before triggering the interrupt)
    cpu->signal_vdd(Edge::Front::RISING);
    interrupt_at_start->signal_vdd(Edge::Front::RISING);

    scheduler.add(cpu);
    scheduler.add(clock);

    memory_view.set_rom(rom, std::min(rom_data.size(), static_cast<size_t>(0x1000)), 0x0000);
    memory_view.set_ram(ram, RAM_SIZE, RAM_START);
}

void Simulator::step(float average_frame_time_in_ms, ControllerWidget::State controller_state)
{
    if (controller_state != ControllerWidget::PAUSED)
    {
        if (controller_state == ControllerWidget::RUNNING ||
            controller_state == ControllerWidget::STEP_ONE_FRAME)
        {
            auto start_point = scheduler.get_counter();

            float time_to_simulation = average_frame_time_in_ms;
            if (average_frame_time_in_ms > 17.f)
            {
                time_to_simulation = (16.6f / average_frame_time_in_ms) * 16.f;
            }
            auto time_in_ns = time_to_simulation * 1000.f * 1000.f;

            uint64_t end_point = start_point + (static_cast<uint64_t>(time_in_ns));

            while (scheduler.get_counter() < end_point)
            {
                scheduler.step();
            }
        }
        else if (controller_state == ControllerWidget::STEP_ONE_STATE)
        {
            auto initial_state = get_cpu().get_output_pins().state;

            while (get_cpu().get_output_pins().state == initial_state)
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
        else if (controller_state == ControllerWidget::STEP_ONE_INSTRUCTION)
        {
            while (get_cpu().get_debug_data().cycle_control == Constants8008::CycleControl::PCI &&
                   (get_cpu().get_output_pins().state == CPU8008::CpuState::T1))
            {
                scheduler.step();
            }

            while (!((get_cpu().get_debug_data().cycle_control ==
                      Constants8008::CycleControl::PCI) &&
                     (get_cpu().get_output_pins().state == CPU8008::CpuState::T1)))
            {
                scheduler.step();
            }
        }
    }
}

const Scheduler& Simulator::get_scheduler() const { return scheduler; }
const CPU8008& Simulator::get_cpu() const { return *cpu; }
const DataBus& Simulator::get_data_bus() const { return *data_bus; }
const MemoryView& Simulator::get_memory_view() { return memory_view; }

void SimulatorMemoryView::set_rom(std::shared_ptr<SimpleROM> rom, std::size_t size,
                                  uint16_t start_address)
{
    this->rom = std::move(rom);
    rom_size = size;
    rom_start_address = start_address;
}

void SimulatorMemoryView::set_ram(std::shared_ptr<SimpleRAM> ram, std::size_t size,
                                  uint16_t start_address)
{
    this->ram = std::move(ram);
    ram_size = size;
    ram_start_address = start_address;
}

uint8_t SimulatorMemoryView::get(std::uint16_t address) const
{
    if (address >= rom_start_address && address < rom_start_address + rom_size)
    {
        return rom->get_direct_data(address - rom_start_address);
    }
    else if (address >= ram_start_address && address < ram_start_address + ram_size)
    {
        return ram->get_direct_data(address - ram_start_address);
    }
    return 0;
}

size_t SimulatorMemoryView::size() const { return 0x4000; }
