
#ifndef MICRALN_SIMULATOR_H
#define MICRALN_SIMULATOR_H

#include "ControllerWidget.h"

#include <devices/src/DoubleClock.h>
#include <emulation_core/src/Scheduler.h>
#include <gui/src/lib/SignalRecorder.h>

class SimpleROM;
class CPU8008;
class ControlBus;
class InterruptAtStart;

class Simulator
{
public:
    Simulator();
    void step(float average_frame_time, ControllerWidget::State controller_state);

    [[nodiscard]] const Scheduler & get_scheduler() const;
    [[nodiscard]] const CPU8008 & get_cpu() const;

    // To be extracted nicely.
    const size_t SIGNAL_RECORDER_WINDOW = 40;
    SignalRecorder phase_1_recorder{SIGNAL_RECORDER_WINDOW};
    SignalRecorder phase_2_recorder{SIGNAL_RECORDER_WINDOW};
    SignalRecorder sync_recorder{SIGNAL_RECORDER_WINDOW};

    uint64_t clock_1_pulse{};
    uint64_t clock_2_pulse{};
private:
    Scheduler scheduler{};

    std::shared_ptr<CPU8008> cpu{};
    std::shared_ptr<SimpleROM> rom{};
    std::shared_ptr<ControlBus> control_bus{};
    std::shared_ptr<InterruptAtStart> interrupt_at_start;
};

#endif //MICRALN_SIMULATOR_H
