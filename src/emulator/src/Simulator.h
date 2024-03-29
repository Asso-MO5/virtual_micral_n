#ifndef MICRALN_SIMULATOR_H
#define MICRALN_SIMULATOR_H

#include "RecorderCollection.h"

#include <devices/src/DoubleClock.h>
#include <devices/src/MemoryCard.h>
#include <emulation_core/src/Scheduler.h>
#include <emulator/src/SignalRecorder.h>
#include <emulator/src/SimulationRunType.h>
#include <i8008/src/MemoryView.h>

class AutomaticStart;
class ConsoleCard;
class DiskControllerCard;
class IOCard;
class MemoryCard;
class Pluribus;
class ProcessorCard;
class SerialCard;
class StackChannelCard;
class VirtualDisk;
class VirtualTTY;

namespace Connectors
{
    class SerialCard_To_IOCard;
    class StackChannel_To_DiskController;
}

class SimulatorMemoryView : public MemoryView
{
public:
    void add_memory_card(const std::shared_ptr<MemoryCard>& memory_card);

    [[nodiscard]] uint8_t get(std::uint16_t address) const override;
    [[nodiscard]] size_t size() const override;

private:
    std::vector<std::shared_ptr<MemoryCard>> memory_cards;
};

enum ConfigROM
{
    HARD_CODED,
    LOOP_LOADS,
    INPUT_OUTPUT,
    HELLO_WORLD,
    HELLO_MO5,
    BANNER_MO5,
    MICRAL_MIC_1,
    MICRAL_38_3F,
    MICRAL_N,
};

class Simulator
{
public:
    explicit Simulator(ConfigROM rom_config);
    void step(float average_frame_time_in_ms, SimulationRunType controller_state);

    [[nodiscard]] const Scheduler& get_scheduler() const;
    [[nodiscard]] const ProcessorCard& get_processor_card() const;
    [[nodiscard]] const Pluribus& get_pluribus() const;
    [[nodiscard]] const StackChannelCard& get_stack_channel_card(int card_number) const;
    [[nodiscard]] const DiskControllerCard& get_disk_controller_card() const;
    [[nodiscard]] const VirtualTTY & get_virtual_tty() const;

    const MemoryView& get_memory_view() const;
    const RecorderCollection& get_recorders() const;

    ConsoleCard& get_console_card();

private:
    Scheduler scheduler{};

    std::shared_ptr<MemoryCard> memory_card_1;
    std::shared_ptr<MemoryCard> memory_card_2;
    std::shared_ptr<Pluribus> pluribus;
    std::shared_ptr<ConsoleCard> console_card;
    std::shared_ptr<ProcessorCard> processor_card;
    std::shared_ptr<StackChannelCard> stack_channel_5_card;
    std::shared_ptr<StackChannelCard> stack_channel_6_card;
    std::shared_ptr<IOCard> io_card;
    std::shared_ptr<SerialCard> serial_card;
    std::shared_ptr<VirtualTTY> virtual_tty;
    std::shared_ptr<DiskControllerCard> disk_controller_card;
    std::shared_ptr<Connectors::StackChannel_To_DiskController>
            stackchannel_diskcontroller_connector; // Not sure it is necessary to keep it
    std::shared_ptr<Connectors::SerialCard_To_IOCard>
            serial_io_connector; // Not sure it is necessary to keep it

    std::shared_ptr<VirtualDisk> virtual_disk;

    SimulatorMemoryView memory_view;
    RecorderCollection recorders;

    // The mask is under the form s13 s12 s11 as the 3 lower bits.
    MemoryCard::Config get_memory_card_rom_2k_config(uint8_t mask);
    MemoryCard::Config get_memory_card_ram_2k_config(uint8_t mask);
    MemoryCard::Config get_memory_card_masked_rom_ram_4k_config(uint8_t mask);

    void connect_signal_recorders();
    void connect_value_recorders();
    void pause_all_recorders();
    void resume_all_recorders();

    void create_virtual_disk();
    void create_processor_card();
    void create_memory_cards(ConfigROM rom_config);
    void create_stack_card();
    void create_disk_system();
    void create_serial_system();
    void create_console_card();
    void add_ram_card(uint8_t mask);
};

#endif //MICRALN_SIMULATOR_H
