#include "Simulator.h"

#include "VirtualDisk.h"
#include "VirtualTTY.h"

#include <devices/src/CPU8008.h>
#include <devices/src/Clock.h>
#include <devices/src/ConsoleCard.h>
#include <devices/src/DiskControllerCard.h>
#include <devices/src/IOCard.h>
#include <devices/src/MemoryCard.h>
#include <devices/src/Pluribus.h>
#include <devices/src/ProcessorCard.h>
#include <devices/src/SerialCard.h>
#include <devices/src/StackChannelCard.h>
#include <devices/src/connectors/SerialCard_To_IOCard.h>
#include <devices/src/connectors/StackChannel_To_DiskController.h>
#include <file_utils/src/FileReader.h>

namespace
{
    std::vector<uint8_t> get_rom_data(ConfigROM rom_config)
    {
        std::filesystem::path data_path("data");
        switch (rom_config)
        {
            case HARD_CODED:
                return {0xc0, 0x2e, 0xff, 0x2e, 0x00, 0x36, 0xc0,
                        0x36, 0x00, 0xc7, 0x44, 0x00, 0x00};
            case LOOP_LOADS:
                return FileReader(data_path / "8008-loop-loads.bin").data;
            case INPUT_OUTPUT:
                return FileReader(data_path / "8008-input-output.bin").data;
            case HELLO_WORLD:
                return FileReader(data_path / "8008-hello-world.bin").data;
            case HELLO_MO5:
                return FileReader(data_path / "8008-hello-mo5.bin").data;
            case BANNER_MO5:
                return FileReader(data_path / "8008-logo-mo5.bin").data;
            case MICRAL_38_3F:
                return FileReader(data_path / "MIC_38_3F.BIN").data;
            case MICRAL_MIC_1:
                return FileReader(data_path / "MIC_1_EPROM_CARTE_MEM_4K.BIN").data;
            case MICRAL_N:
                assert(false && "MICRAL_N is not a configuration to load a specific ROM.");
                return {};
        }
        return {};
    }
}

Simulator::Simulator(ConfigROM rom_config)
{
    create_virtual_disk();

    pluribus = std::make_shared<Pluribus>();

    create_processor_card();
    create_memory_cards(rom_config);
    create_console_card();
    create_stack_card();
    create_serial_system();
    create_disk_system();

    connect_signal_recorders();
    connect_value_recorders();
    pause_all_recorders();
    resume_all_recorders();

    pluribus->vdd.request(this);
    pluribus->vdd.set(State{State::HIGH}, Scheduling::counter_type{0}, this);
}

void Simulator::create_console_card()
{
    console_card = std::make_shared<ConsoleCard>(
            pluribus, [&](Scheduling::schedulable_id id) { scheduler.change_schedule(id); },
            ConsoleCard::Automatic, ConsoleCard::Record);
    scheduler.add(console_card);
}

void Simulator::create_serial_system()
{
    IOCard::Config io_card_config{
            .change_schedule =
                    [&](Scheduling::schedulable_id id) { scheduler.change_schedule(id); },
            .pluribus = pluribus,
            .configuration = {
                    .mode = IOCardConfiguration::Input_32_Output_32,
                    .address_selection = 0b01000000, // 010 for Output, 0 for Input
            }};
    io_card = std::make_shared<IOCard>(io_card_config);
    scheduler.add(io_card);

    virtual_tty = std::make_shared<VirtualTTY>();

    SerialCard::Config serial_card_config{
            .change_schedule =
                    [&](Scheduling::schedulable_id id) { scheduler.change_schedule(id); },
            .pluribus = pluribus,
            .configuration = {.on_output_character = [this](char character) {
                virtual_tty->receive_char(character);
            }}};
    serial_card = std::make_shared<SerialCard>(serial_card_config);
    scheduler.add(serial_card);

    serial_io_connector =
            std::make_shared<Connectors::SerialCard_To_IOCard>(*serial_card, *io_card);

    virtual_tty->set_emitted_char_cb(
            [this](char emitted_char) { serial_card->add_input(emitted_char); });
}

void Simulator::create_disk_system()
{
    StackChannelCard::Config stack_channel_5_config{
            .change_schedule =
                    [&](Scheduling::schedulable_id id) { scheduler.change_schedule(id); },
            .pluribus = pluribus,
            .configuration = {
                    .mode = StackChannelCardConfiguration::Channel,
                    .memory_size = 256,
                    .input_address = 0x05,
                    .output_address = 0x15,
            }};
    stack_channel_5_card = std::make_shared<StackChannelCard>(stack_channel_5_config);
    scheduler.add(stack_channel_5_card);

    DiskControllerCard::Config disk_controller_card_config{
            .change_schedule =
                    [&](Scheduling::schedulable_id id) { scheduler.change_schedule(id); },
            .pluribus = pluribus,
            .configuration = {
                    .address_selection = 0b10100001, // 101 for Output, 1 for Input
                    .data_provider =
                            [this](DiskReader::track_type track, DiskReader::sector_type sector,
                                   size_t index) {
                                return virtual_disk->get(track, sector, index);
                            },
            }};
    disk_controller_card = std::make_shared<DiskControllerCard>(disk_controller_card_config);
    scheduler.add(disk_controller_card);

    // Connection between the Stack Channel and the Disk Controller
    stackchannel_diskcontroller_connector =
            std::make_shared<Connectors::StackChannel_To_DiskController>(*stack_channel_5_card,
                                                                         *disk_controller_card);
}

void Simulator::create_stack_card()
{
    StackChannelCard::Config stack_channel_6_config{
            .change_schedule =
                    [&](Scheduling::schedulable_id id) { scheduler.change_schedule(id); },
            .pluribus = pluribus,
            .configuration = {
                    .mode = StackChannelCardConfiguration::Stack,
                    .memory_size = 256,
                    .input_address = 0x06,
                    .output_address = 0x16,
            }};
    stack_channel_6_card = std::make_shared<StackChannelCard>(stack_channel_6_config);
    scheduler.add(stack_channel_6_card);
}

void Simulator::create_memory_cards(ConfigROM rom_config)
{
    if (rom_config == ConfigROM::MICRAL_N)
    {
        auto boot_rom_data = get_rom_data(ConfigROM::MICRAL_MIC_1);
        MemoryCard::Config masked_rom_memory_config =
                get_memory_card_masked_rom_ram_4k_config(0b000);
        memory_card_1 = std::make_shared<MemoryCard>(masked_rom_memory_config);
        memory_card_1->load_data(boot_rom_data);
        scheduler.add(memory_card_1);
        memory_view.add_memory_card(memory_card_1);

        auto monitor_rom_data = get_rom_data(ConfigROM::MICRAL_38_3F);
        MemoryCard::Config rom_memory_config = get_memory_card_rom_2k_config(0b111);
        memory_card_2 = std::make_shared<MemoryCard>(rom_memory_config);
        memory_card_2->load_data(monitor_rom_data);
        scheduler.add(memory_card_2);
        memory_view.add_memory_card(memory_card_2);

        // Install RAM Cards
        for (uint8_t mask = 0b010; mask <= 0b110; mask += 1)
        {
            add_ram_card(mask);
        }
    }
    else
    {
        auto rom_data = get_rom_data(rom_config);

        MemoryCard::Config rom_memory_config = get_memory_card_rom_2k_config(0b000);
        memory_card_1 = std::make_shared<MemoryCard>(rom_memory_config);
        memory_card_1->load_data(rom_data);
        scheduler.add(memory_card_1);
        memory_view.add_memory_card(memory_card_1);

        for (uint8_t mask = 0b010; mask <= 0b111; mask += 1)
        {
            add_ram_card(mask);
        }
    }
}

void Simulator::add_ram_card(uint8_t mask)
{
    MemoryCard::Config ram_memory_config = get_memory_card_ram_2k_config(mask);
    auto ram_card = std::make_shared<MemoryCard>(ram_memory_config);
    scheduler.add(ram_card);
    memory_view.add_memory_card(ram_card);
}

void Simulator::create_processor_card()
{
    ProcessorCard::Config processor_card_config{
            .change_schedule =
                    [&](Scheduling::schedulable_id id) { scheduler.change_schedule(id); },
            .pluribus = pluribus,
    };

    processor_card = std::make_shared<ProcessorCard>(processor_card_config);
    processor_card->install_debug_info();

    scheduler.add(processor_card);
}

void Simulator::create_virtual_disk()
{
    std::filesystem::path data_path("data");
    auto disk_data = FileReader(data_path / "8008-logo-mo5.bin").data;
    // The data starts at Track 0 Sector 16
    disk_data.insert(begin(disk_data), 16 * 128, 0xaa);
    virtual_disk = std::make_shared<VirtualDisk>(
            disk_data, VirtualDisk::Layout{.tracks = 10, .sectors = 32, .sector_size = 128});
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
    auto& substitution_recorder =
            recorders.create_and_get_signal_recorder("SUB", window_time_frame_in_s, 50 * 4);
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
    connect_recorder(pluribus->sub, substitution_recorder);

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

MemoryCard::Config Simulator::get_memory_card_masked_rom_ram_4k_config(uint8_t mask)
{
    auto construction_config = MemoryCard::Config{
            .change_schedule =
                    [&](Scheduling::schedulable_id id) { scheduler.change_schedule(id); },
            .pluribus = pluribus,
            .configuration = {.addressing_size = MemoryCardConfiguration::Card4k,
                              .access_type = MemoryCardConfiguration::ROM_RAM_256,
                              .selection_mask = mask}};
    return construction_config;
}

MemoryCard::Config Simulator::get_memory_card_rom_2k_config(uint8_t mask)
{
    auto construction_config = MemoryCard::Config{
            .change_schedule =
                    [&](Scheduling::schedulable_id id) { scheduler.change_schedule(id); },
            .pluribus = pluribus,
            .configuration = {.addressing_size = MemoryCardConfiguration::Card2k,
                              .access_type = MemoryCardConfiguration::ROM,
                              .selection_mask = mask}};
    return construction_config;
}

MemoryCard::Config Simulator::get_memory_card_ram_2k_config(uint8_t mask)
{
    auto construction_config = MemoryCard::Config{
            .change_schedule =
                    [&](Scheduling::schedulable_id id) { scheduler.change_schedule(id); },
            .pluribus = pluribus,
            .configuration = {
                    .addressing_size = MemoryCardConfiguration::Card2k,
                    .access_type = MemoryCardConfiguration::RAM,
                    .selection_mask = mask,
            }};
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

            auto end_point = start_point + Scheduling::counter_type(time_in_ns);

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
const MemoryView& Simulator::get_memory_view() const { return memory_view; }
const ProcessorCard& Simulator::get_processor_card() const { return *processor_card; }
ConsoleCard& Simulator::get_console_card() { return *console_card; }
const RecorderCollection& Simulator::get_recorders() const { return recorders; }
const Pluribus& Simulator::get_pluribus() const { return *pluribus; }

const StackChannelCard& Simulator::get_stack_channel_card(int card_number) const
{
    return card_number == 0 ? *stack_channel_5_card : *stack_channel_6_card;
}

const DiskControllerCard& Simulator::get_disk_controller_card() const
{
    return *disk_controller_card;
}

const VirtualTTY& Simulator::get_virtual_tty() const { return *virtual_tty; }

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
                return memory->get_data_at(address);
            }
        }
    }
    return 0;
}

size_t SimulatorMemoryView::size() const { return 0x4000; }
