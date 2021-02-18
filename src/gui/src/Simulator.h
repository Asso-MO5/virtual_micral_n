
#ifndef MICRALN_SIMULATOR_H
#define MICRALN_SIMULATOR_H

#include "ControllerWidget.h"

#include <devices/src/DoubleClock.h>
#include <devices/src/MemoryView.h>
#include <devices/src/SimpleRAM.h>
#include <emulation_core/src/DataBus.h>
#include <emulation_core/src/Scheduler.h>
#include <gui/src/lib/SignalRecorder.h>
#include <span>

class SimpleROM;
class CPU8008;
class ControlBus;
class InterruptAtStart;

class SimulatorMemoryView : public MemoryView
{
public:
    void set_rom(std::shared_ptr<SimpleROM> rom, std::size_t size, uint16_t start_address);
    void set_ram(std::shared_ptr<SimpleRAM> ram, std::size_t size, uint16_t start_address);

    [[nodiscard]] uint8_t get(std::uint16_t address) const override;
    [[nodiscard]] size_t size() const override;

private:
    std::shared_ptr<SimpleROM> rom{};
    std::size_t rom_size;
    uint16_t rom_start_address;

    std::shared_ptr<SimpleRAM> ram{};
    std::size_t ram_size;
    uint16_t ram_start_address;
};

class Simulator
{
public:
    Simulator();
    void step(float average_frame_time_in_ms, ControllerWidget::State controller_state);

    [[nodiscard]] const Scheduler& get_scheduler() const;
    [[nodiscard]] const CPU8008& get_cpu() const;
    [[nodiscard]] const DataBus& get_data_bus() const;

    // To be extracted nicely.
    const size_t SIGNAL_RECORDER_WINDOW = 40;
    SignalRecorder phase_1_recorder{SIGNAL_RECORDER_WINDOW};
    SignalRecorder phase_2_recorder{SIGNAL_RECORDER_WINDOW};
    SignalRecorder sync_recorder{SIGNAL_RECORDER_WINDOW};

    uint64_t clock_1_pulse{};
    uint64_t clock_2_pulse{};

    const MemoryView& get_memory_view();

private:
    Scheduler scheduler{};

    std::shared_ptr<CPU8008> cpu{};
    std::shared_ptr<SimpleROM> rom{};
    std::shared_ptr<SimpleRAM> ram{};
    std::shared_ptr<ControlBus> control_bus{};
    std::shared_ptr<DataBus> data_bus{};
    std::shared_ptr<InterruptAtStart> interrupt_at_start;

    SimulatorMemoryView memory_view;
};

#endif //MICRALN_SIMULATOR_H
