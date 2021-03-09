#ifndef MICRALN_SIMULATOR_H
#define MICRALN_SIMULATOR_H

#include "ControllerWidget.h"
#include "RecorderCollection.h"

#include <devices/src/DoubleClock.h>
#include <devices/src/MemoryCard.h>
#include <devices/src/MemoryView.h>
#include <emulation_core/src/Scheduler.h>
#include <gui/src/lib/SignalRecorder.h>

class DataBus;
class IOController;
class ConsoleCard;
class ProcessorCard;
class Pluribus;
class InterruptAtStart;
class MemoryCard;

class SimulatorMemoryView : public MemoryView
{
public:
    void add_memory_card(const std::shared_ptr<MemoryCard>& memory_card);

    [[nodiscard]] uint8_t get(std::uint16_t address) const override;
    [[nodiscard]] size_t size() const override;

private:
    std::vector<std::shared_ptr<MemoryCard>> memory_cards;
};

class Simulator
{
public:
    Simulator();
    void step(float average_frame_time_in_ms, ControllerWidget::State controller_state);

    [[nodiscard]] const Scheduler& get_scheduler() const;
    [[nodiscard]] const DataBus& get_data_bus() const;
    [[nodiscard]] IOController& get_io_controller();
    [[nodiscard]] const ProcessorCard& get_processor_card() const;

    // TODO: remove once the signals go through the Pluribus
    [[nodiscard]] ProcessorCard& get_processor_card();

    const MemoryView& get_memory_view();
    const RecorderCollection& get_recorders() const;

    ConsoleCard& get_console_card();

private:
    Scheduler scheduler{};

    std::shared_ptr<MemoryCard> memory_card_1;
    std::shared_ptr<MemoryCard> memory_card_2;
    std::shared_ptr<IOController> io_controller;
    std::shared_ptr<Pluribus> pluribus;
    std::shared_ptr<ConsoleCard> console_card;
    std::shared_ptr<ProcessorCard> processor_card;

    SimulatorMemoryView memory_view;
    RecorderCollection recorders;

    MemoryCard::Config get_memory_card_rom_2k_config(bool s13, bool s12, bool s11);
    MemoryCard::Config get_memory_card_ram_2k_config(bool s13, bool s12, bool s11);
    void register_signals();
};

#endif //MICRALN_SIMULATOR_H
