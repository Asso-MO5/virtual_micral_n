
#include "Simulator.h"
#include "ControllerWidget.h"

#include <devices/src/CPU8008.h>
#include <devices/src/ConsoleCard.h>
#include <devices/src/IOController.h>
#include <devices/src/MemoryCard.h>
#include <devices/src/Pluribus.h>
#include <devices/src/ProcessorCard.h>
#include <emulation_core/src/DataBus.h>
#include <fstream>

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
    ReadRomData rom_data_file("data/8008-loop-loads.bin");
    auto& rom_data = rom_data_file.data;

    //    ReadRomData rom_data_file("data/8008-input-output.bin");
    //    auto& rom_data = rom_data_file.data;

    //    ReadRomData rom_data_file("data/8008-hello-world.bin");
    //    auto& rom_data = rom_data_file.data;

    //    std::vector<uint8_t> rom_data{0xc0, 0x2e, 0xff, 0x2e, 0x00, 0x36, 0xc0,
    //                                  0x36, 0x00, 0xc7, 0x44, 0x00, 0x00};

    // Simulation Setup
    pluribus = std::make_shared<Pluribus>();
    auto& data_bus_d0_7 = pluribus->data_bus_d0_7;

    ProcessorCard::Config processor_card_config{
            .scheduler = scheduler,
            .pluribus = pluribus,
    };

    processor_card = std::make_shared<ProcessorCard>(processor_card_config);
    processor_card->install_debug_info();
    processor_card->connect_data_bus(data_bus_d0_7);

    MemoryCard::Config rom_memory_config = get_memory_card_rom_2k_config(false, false, false);
    memory_card_1 = std::make_shared<MemoryCard>(rom_memory_config);
    memory_card_1->load_data(rom_data);

    MemoryCard::Config ram_memory_config = get_memory_card_ram_2k_config(false, true, false);
    memory_card_2 = std::make_shared<MemoryCard>(ram_memory_config);

    io_controller = std::make_shared<IOController>(processor_card->get_cpu(), data_bus_d0_7);
    console_card = std::make_shared<ConsoleCard>(pluribus);

    auto& clock = processor_card->get_clock();
    clock.phase_1.subscribe([this](Edge edge) { io_controller->signal_phase_1(edge); });
    clock.phase_2.subscribe([this](Edge edge) { io_controller->signal_phase_2(edge); });
    pluribus->sync.subscribe([this](Edge edge) { io_controller->signal_sync(edge); });

    register_signals();
    pause_all_recorders();
    resume_all_recorders();

    for (auto& sub : processor_card->get_sub_schedulables())
    {
        scheduler.add(sub);
    }
    scheduler.add(processor_card);
    scheduler.add(console_card);
    scheduler.add(memory_card_1);
    scheduler.add(memory_card_2);

    memory_view.add_memory_card(memory_card_1);
    memory_view.add_memory_card(memory_card_2);

    pluribus->vdd.request(this);
    pluribus->vdd.set(State{State::HIGH}, Scheduling::counter_type{0}, this);
}

void connect_recorder(OwnedSignal& signal, SignalRecorder& recorder)
{
    signal.subscribe([&recorder](Edge edge) { recorder.add(edge); });
}

void Simulator::register_signals()
{
    auto& clock = processor_card->get_clock();

    const double window_time_frame_in_s = 20.f / 1000.f / 1000.f;
    auto& ready_recorder = recorders.create_and_get("READY", window_time_frame_in_s, 300'000 * 4);
    auto& ready_console_recorder =
            recorders.create_and_get("READY C", window_time_frame_in_s, 300'000 * 4);
    auto& stop_recorder = recorders.create_and_get("STOP", window_time_frame_in_s, 300'000 * 4);
    auto& wait_recorder = recorders.create_and_get("WAIT", window_time_frame_in_s, 300'000 * 4);
    auto& cc0_recorder = recorders.create_and_get("CC0", window_time_frame_in_s, 300'000 * 4);
    auto& cc1_recorder = recorders.create_and_get("CC1", window_time_frame_in_s, 300'000 * 4);
    auto& t3prime_recorder = recorders.create_and_get("T'3", window_time_frame_in_s, 300'000 * 4);
    auto& t3_recorder = recorders.create_and_get("T3", window_time_frame_in_s, 300'000 * 4);
    auto& t2_recorder = recorders.create_and_get("T2", window_time_frame_in_s, 300'000 * 4);
    auto& sync_recorder = recorders.create_and_get("Sync", window_time_frame_in_s, 300'000 * 4);
    auto& phase_2_recorder =
            recorders.create_and_get("Phase 2", window_time_frame_in_s, 550'000 * 4);
    auto& phase_1_recorder =
            recorders.create_and_get("Phase 1", window_time_frame_in_s, 550'000 * 4);

    connect_recorder(clock.phase_1, phase_1_recorder);
    connect_recorder(clock.phase_2, phase_2_recorder);
    connect_recorder(pluribus->t3, t3_recorder);
    connect_recorder(pluribus->t2, t2_recorder);
    connect_recorder(pluribus->sync, sync_recorder);
    connect_recorder(pluribus->t3prime, t3prime_recorder);

    connect_recorder(pluribus->cc0, cc0_recorder);
    connect_recorder(pluribus->cc1, cc1_recorder);
    connect_recorder(pluribus->wait, wait_recorder);
    connect_recorder(pluribus->stop, stop_recorder);

    connect_recorder(pluribus->ready, ready_recorder);
    connect_recorder(pluribus->ready_console, ready_console_recorder);
}

void Simulator::pause_all_recorders()
{
    for (auto& recorder : recorders)
    {
        recorder.second.pause();
    }
}

void Simulator::resume_all_recorders()
{
    for (auto& recorder : recorders)
    {
        recorder.second.resume();
    }
}


MemoryCard::Config Simulator::get_memory_card_rom_2k_config(bool s13, bool s12, bool s11)
{
    auto configuration = MemoryCardConfiguration{
            .addressing_size = MemoryCard::AddressingSize::Card2k,
            .writable_page =
                    {
                            false,
                            false,
                            false,
                            false,
                            false,
                            false,
                            false,
                            false,
                    },
            .selection_mask = {s13, s12, s11},
    };
    auto construction_config = MemoryCard::Config{.scheduler = scheduler,
                                                  .pluribus = pluribus,
                                                  .configuration = configuration};
    return construction_config;
}

MemoryCard::Config Simulator::get_memory_card_ram_2k_config(bool s13, bool s12, bool s11)
{
    auto configuration = MemoryCardConfiguration{
            .addressing_size = MemoryCard::AddressingSize::Card2k,
            .writable_page =
                    {
                            true,
                            true,
                            true,
                            true,
                            true,
                            true,
                            true,
                            true,
                    },
            .selection_mask = {s13, s12, s11},
    };

    auto construction_config = MemoryCard::Config{
            .scheduler = scheduler,
            .pluribus = pluribus,
            .configuration = configuration,
    };
    return construction_config;
}

void Simulator::step(float average_frame_time_in_ms, ControllerWidget::State controller_state)
{
    if (controller_state != ControllerWidget::PAUSED)
    {
        auto& cpu = processor_card->get_cpu();

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
            auto initial_state = *cpu.get_output_pins().state;

            int timeout = 0;
            if (initial_state == Constants8008::CpuState::STOPPED)
            {
                timeout = 50;
            }

            while (*cpu.get_output_pins().state == initial_state)
            {
                scheduler.step();

                if (timeout > 0)
                {
                    timeout -= 1;
                    if (timeout == 0)
                    {
                        break;
                    }
                }
            }
        }
        else if (controller_state == ControllerWidget::STEP_ONE_CLOCK)
        {
            auto& clock = processor_card->get_clock();
            auto initial_clock_1 = clock.get_phase_1_state();
            auto initial_clock_2 = clock.get_phase_2_state();

            while (initial_clock_1 == clock.get_phase_1_state() &&
                   initial_clock_2 == clock.get_phase_2_state())
            {
                scheduler.step();
            }
        }
        else if (controller_state == ControllerWidget::STEP_ONE_INSTRUCTION)
        {
            while (cpu.get_debug_data().cycle_control == Constants8008::CycleControl::PCI &&
                   (*cpu.get_output_pins().state == Constants8008::CpuState::T1))
            {
                scheduler.step();
            }

            while (!((cpu.get_debug_data().cycle_control == Constants8008::CycleControl::PCI) &&
                     (*cpu.get_output_pins().state == Constants8008::CpuState::T1)))
            {
                scheduler.step();
            }
        }
    }
}

const Scheduler& Simulator::get_scheduler() const { return scheduler; }

const DataBus& Simulator::get_data_bus() const { return *pluribus->data_bus_d0_7; }
const MemoryView& Simulator::get_memory_view() { return memory_view; }
IOController& Simulator::get_io_controller() { return *io_controller; }
const ProcessorCard& Simulator::get_processor_card() const { return *processor_card; }
ProcessorCard& Simulator::get_processor_card() { return *processor_card; }

ConsoleCard& Simulator::get_console_card() { return *console_card; }
const RecorderCollection& Simulator::get_recorders() const { return recorders; }

void SimulatorMemoryView::add_memory_card(const std::shared_ptr<MemoryCard>& memory_card)
{
    memory_cards.push_back(memory_card);

    std::sort(begin(memory_cards), end(memory_cards), [](const auto& a, const auto& b) {
        return a->get_start_address() < b->get_start_address();
    });
}

uint8_t SimulatorMemoryView::get(std::uint16_t address) const
{
    for (const auto& memory : memory_cards)
    {
        auto start_address = memory->get_start_address();

        if (address >= start_address)
        {
            auto end_address = start_address + memory->get_length();

            if (address < end_address)
            {
                return memory->get_data_at(address - start_address);
            }
        }
    }
    return 0;
}

size_t SimulatorMemoryView::size() const { return 0x4000; }
