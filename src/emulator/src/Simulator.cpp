#include "Simulator.h"

#include <devices/src/CPU8008.h>
#include <devices/src/Clock.h>
#include <devices/src/ConsoleCard.h>
#include <devices/src/IOCard.h>
#include <devices/src/MemoryCard.h>
#include <devices/src/Pluribus.h>
#include <devices/src/ProcessorCard.h>
#include <devices/src/StackChannelCard.h>
#include <devices/src/UnknownCard.h>

#include <fstream>

namespace
{
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

    std::vector<uint8_t> get_rom_data(ConfigROM& rom_config)
    {
        switch (rom_config)
        {
            case HARD_CODED:
                return {0xc0, 0x2e, 0xff, 0x2e, 0x00, 0x36, 0xc0,
                        0x36, 0x00, 0xc7, 0x44, 0x00, 0x00};
            case LOOP_LOADS:
                return ReadRomData("data/8008-loop-loads.bin").data;
            case INPUT_OUTPUT:
                return ReadRomData("data/8008-input-output.bin").data;
            case HELLO_WORLD:
                return ReadRomData("data/8008-hello-world.bin").data;
            case MICRAL_38_3F:
                return ReadRomData("data/MIC_38_3F.BIN").data;
            case MICRAL_MIC_1:
                return ReadRomData("data/MIC_1_EPROM_CARTE_MEM_4K.BIN").data;
        }
        return {};
    }
} // namespace

Simulator::Simulator(ConfigROM rom_config)
{
    auto rom_data = get_rom_data(rom_config);

    // Simulation Setup
    pluribus = std::make_shared<Pluribus>();

    ProcessorCard::Config processor_card_config{
            .scheduler = scheduler,
            .pluribus = pluribus,
    };

    processor_card = std::make_shared<ProcessorCard>(processor_card_config);
    processor_card->install_debug_info();

    // MemoryCard::Config rom_memory_config = get_memory_card_rom_2k_config(true, true, true);
    MemoryCard::Config rom_memory_config = get_memory_card_rom_2k_config(false, false, false);
    memory_card_1 = std::make_shared<MemoryCard>(rom_memory_config);
    memory_card_1->load_data(rom_data);

    MemoryCard::Config ram_memory_config = get_memory_card_ram_2k_config(false, true, false);
    memory_card_2 = std::make_shared<MemoryCard>(ram_memory_config);

    console_card = std::make_shared<ConsoleCard>(pluribus, ConsoleCard::StartMode::Manual,
                                                 ConsoleCard::RecordMode::Record);

    StackChannelCard::Config stack_channel_6_config{
            .scheduler = scheduler,
            .pluribus = pluribus,
            .configuration = {
                    .mode = StackChannelCardConfiguration::Stack,
                    .memory_size = 256,
                    .input_address = 0x06,
                    .output_address = 0x16,
            }};
    stack_channel_6_card = std::make_shared<StackChannelCard>(stack_channel_6_config);

    StackChannelCard::Config stack_channel_5_config{
            .scheduler = scheduler,
            .pluribus = pluribus,
            .configuration = {
                    .mode = StackChannelCardConfiguration::Stack,
                    .memory_size = 256,
                    .input_address = 0x05,
                    .output_address = 0x15,
            }};
    stack_channel_5_card = std::make_shared<StackChannelCard>(stack_channel_5_config);

    IOCard::Config io_card_config{
            .scheduler = scheduler,
            .pluribus = pluribus,
            .configuration = {
                    .mode = IOCardConfiguration::Input_32_Output_32,
                    .address_selection = 0b10100001, // 101 for Output, 1 for Input
                                                     // This correspond to usage from the Boot ROM.
            }};
    io_card = std::make_shared<IOCard>(io_card_config);

    UnknownCard::Config unknown_card_config{.scheduler = scheduler,
                                            .io_card = io_card,
                                            .stack_channel = stack_channel_5_card,
                                            .configuration = {}};
    unknown_card = std::make_shared<UnknownCard>(unknown_card_config);

    connect_signal_recorders();
    connect_value_recorders();
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
    scheduler.add(stack_channel_6_card);
    scheduler.add(stack_channel_5_card);
    scheduler.add(io_card);
    scheduler.add(unknown_card);

    memory_view.add_memory_card(memory_card_1);
    memory_view.add_memory_card(memory_card_2);

    pluribus->vdd.request(this);
    pluribus->vdd.set(State{State::HIGH}, Scheduling::counter_type{0}, this);
}

void connect_recorder(OwnedSignal& signal, SignalRecorder& recorder)
{
    signal.subscribe([&recorder](Edge edge) { recorder.add(edge); });
}

template<typename ValueType>
void connect_recorder(OwnedValue<ValueType>& value, ValueRecorder& recorder)
{
    value.subscribe([&recorder](ValueType, ValueType new_value, Scheduling::counter_type time) {
        recorder.add(new_value, time);
    });

    value.subscribe_to_owner([&recorder](void*, void* new_owner, Scheduling::counter_type time) {
        recorder.change_owner(new_owner, time);
    });
}

void Simulator::connect_signal_recorders()
{
    auto& clock = processor_card->get_clock();
    auto& rtc = processor_card->get_rtc();

    const double window_time_frame_in_s = 20.f / 1000.f / 1000.f;
    auto& aint7_recorder =
            recorders.create_and_get_signal_recorder("AINT7", window_time_frame_in_s, 50 * 4);
    auto& bi7_recorder =
            recorders.create_and_get_signal_recorder("BI7", window_time_frame_in_s, 50 * 4);
    auto& rzgi_recorder =
            recorders.create_and_get_signal_recorder("RZGI", window_time_frame_in_s, 50 * 4);
    auto& init_recorder =
            recorders.create_and_get_signal_recorder("INIT", window_time_frame_in_s, 50 * 4);
    auto& ready_recorder =
            recorders.create_and_get_signal_recorder("READY", window_time_frame_in_s, 300'000 * 4);
    auto& ready_console_recorder = recorders.create_and_get_signal_recorder(
            "READY C", window_time_frame_in_s, 300'000 * 4);
    auto& stop_recorder =
            recorders.create_and_get_signal_recorder("STOP", window_time_frame_in_s, 300'000 * 4);
    auto& wait_recorder =
            recorders.create_and_get_signal_recorder("WAIT", window_time_frame_in_s, 300'000 * 4);
    auto& cc0_recorder =
            recorders.create_and_get_signal_recorder("CC0", window_time_frame_in_s, 300'000 * 4);
    auto& cc1_recorder =
            recorders.create_and_get_signal_recorder("CC1", window_time_frame_in_s, 300'000 * 4);
    auto& t3prime_recorder =
            recorders.create_and_get_signal_recorder("T'3", window_time_frame_in_s, 300'000 * 4);
    auto& t3_recorder =
            recorders.create_and_get_signal_recorder("T3", window_time_frame_in_s, 300'000 * 4);
    auto& t2_recorder =
            recorders.create_and_get_signal_recorder("T2", window_time_frame_in_s, 300'000 * 4);
    auto& sync_recorder =
            recorders.create_and_get_signal_recorder("Sync", window_time_frame_in_s, 300'000 * 4);
    auto& phase_2_recorder = recorders.create_and_get_signal_recorder(
            "Phase 2", window_time_frame_in_s, 550'000 * 4);
    auto& phase_1_recorder = recorders.create_and_get_signal_recorder(
            "Phase 1", window_time_frame_in_s, 550'000 * 4);

    auto& rtc_recorder =
            recorders.create_and_get_signal_recorder("RTC", window_time_frame_in_s, 100 * 2);

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

    connect_recorder(rtc.phase, rtc_recorder);

    connect_recorder(pluribus->init, init_recorder);
    connect_recorder(pluribus->rzgi, rzgi_recorder);

    connect_recorder(pluribus->bi7, bi7_recorder);
    connect_recorder(pluribus->aint7, aint7_recorder);
}

void Simulator::connect_value_recorders()
{
    const double window_time_frame_in_s = 20.f / 1000.f / 1000.f;

    auto& address_bus_recorder = recorders.create_and_get_value_recorder(
            "S0-S13", window_time_frame_in_s, 200'000 * 4, 14, ValueRecorder::DO_NOT_TRACK_OWNERS);
    auto& data_send_bus_recorder = recorders.create_and_get_value_recorder(
            "D0-D7", window_time_frame_in_s, 200'000 * 4, 8, ValueRecorder::DO_NOT_TRACK_OWNERS);
    auto& data_return_bus_recorder = recorders.create_and_get_value_recorder(
            "MD0-MD7", window_time_frame_in_s, 200'000 * 4, 8, ValueRecorder::TRACK_OWNERS);

    connect_recorder(pluribus->address_bus_s0_s13, address_bus_recorder);
    connect_recorder(pluribus->data_bus_d0_7, data_send_bus_recorder);
    connect_recorder(pluribus->data_bus_md0_7, data_return_bus_recorder);
}

void Simulator::pause_all_recorders()
{
    for (auto& recorder : recorders)
    {
        recorder.second->pause();
    }
}

void Simulator::resume_all_recorders()
{
    for (auto& recorder : recorders)
    {
        recorder.second->resume();
    }
}

MemoryCard::Config Simulator::get_memory_card_rom_2k_config(bool s13, bool s12, bool s11)
{
    auto configuration = ::get_rom_2k_configuration(s13, s12, s11);
    auto construction_config = MemoryCard::Config{.scheduler = scheduler,
                                                  .pluribus = pluribus,
                                                  .configuration = configuration};
    return construction_config;
}

MemoryCard::Config Simulator::get_memory_card_ram_2k_config(bool s13, bool s12, bool s11)
{
    auto configuration = ::get_ram_2k_configuration(s13, s12, s11);
    auto construction_config = MemoryCard::Config{
            .scheduler = scheduler,
            .pluribus = pluribus,
            .configuration = configuration,
    };
    return construction_config;
}

void Simulator::step(float average_frame_time_in_ms, SimulationRunType controller_state)
{
    if (controller_state != PAUSED)
    {
        auto& cpu = processor_card->get_cpu();

        if (controller_state == RUNNING || controller_state == RUNNING_SLOW)
        {
            auto start_point = scheduler.get_counter();

            float time_to_simulation = average_frame_time_in_ms;

            if (average_frame_time_in_ms > 17.f)
            {
                time_to_simulation = (16.6f / average_frame_time_in_ms) * 16.f;
            }
            auto time_in_ns = time_to_simulation * 1000.f * 1000.f;

            if (controller_state == RUNNING_SLOW)
            {
                time_in_ns = std::max(time_in_ns / 100.f, 1.f);
            }

            uint64_t end_point = start_point + (static_cast<uint64_t>(time_in_ns));

            while (scheduler.get_counter() < end_point)
            {
                scheduler.step();
            }
        }
        else if (controller_state == RUNNING_VERY_SLOW)
        {
            scheduler.step();
        }
        else if (controller_state == STEP_ONE_STATE)
        {
            auto initial_state = *cpu.output_pins.state;

            int timeout = 50;

            while (*cpu.output_pins.state == initial_state && timeout > 0)
            {
                scheduler.step();
                timeout -= 1;
            }
        }
        else if (controller_state == STEP_ONE_CLOCK)
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
        else if (controller_state == STEP_ONE_INSTRUCTION)
        {
            int timeout = 500;

            while (cpu.get_debug_data().cycle_control == Constants8008::CycleControl::PCI &&
                   (*cpu.output_pins.state == Constants8008::CpuState::T1) && timeout > 0)
            {
                scheduler.step();
                timeout -= 1;
            }

            timeout = 500;

            while (!((cpu.get_debug_data().cycle_control == Constants8008::CycleControl::PCI) &&
                     (*cpu.output_pins.state == Constants8008::CpuState::T1)) &&
                   timeout > 0)
            {
                scheduler.step();
                timeout -= 1;
            }
        }
    }
}

const Scheduler& Simulator::get_scheduler() const { return scheduler; }

const MemoryView& Simulator::get_memory_view() { return memory_view; }
const ProcessorCard& Simulator::get_processor_card() const { return *processor_card; }

ConsoleCard& Simulator::get_console_card() { return *console_card; }
const RecorderCollection& Simulator::get_recorders() const { return recorders; }
const Pluribus& Simulator::get_pluribus() const { return *pluribus; }
const StackChannelCard& Simulator::get_stack_channel_card(int card_number) const
{
    return card_number == 0 ? *stack_channel_5_card : *stack_channel_6_card;
}

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
