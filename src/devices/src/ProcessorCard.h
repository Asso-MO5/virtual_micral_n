#ifndef MICRALN_PROCESSORCARD_H
#define MICRALN_PROCESSORCARD_H

#include "Constants8008.h"
#include "GeneralAddressRegister.h"

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/OwnedSignal.h>
#include <emulation_core/src/Schedulable.h>
#include <memory>

class AutomaticStart;
class CPU8008;
class Clock;
class DataBus;
class DoubleClock;
class InterruptCircuit;
class InterruptController;
class Pluribus;

class ProcessorCard : public SchedulableImpl
{
public:
    struct Config
    {
        SignalReceiver& scheduler;
        std::shared_ptr<Pluribus> pluribus;
    };

    explicit ProcessorCard(Config config);
    ~ProcessorCard() override;

    void step() override;

    [[nodiscard]] const CPU8008& get_cpu() const;

    std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables();

    DoubleClock& get_clock();
    Clock& get_rtc();

    struct DebugInfo
    {
        Scheduling::counter_type clock_pulse{};
        bool watchdog_on{};
    };

    void install_debug_info();
    [[nodiscard]] const DebugInfo& get_debug_info() const;

private:
    // Received as parameters
    std::shared_ptr<Pluribus> pluribus;
    SignalReceiver& scheduler;

    // Constructed devices
    std::shared_ptr<DoubleClock> clock{};
    std::shared_ptr<CPU8008> cpu{};
    std::shared_ptr<InterruptController> interrupt_controller{};
    std::shared_ptr<AutomaticStart> automatic_startup{};
    std::shared_ptr<Clock> real_time_clock{};
    std::unique_ptr<InterruptCircuit> bi7_interrupt_controller{};

    // Inner workings
    OwnedSignal combined_ready{};
    std::shared_ptr<GeneralAddressRegister> bus_address_decoder{};
    bool emit_t3prime_on_next_step{};
    bool t1i_cycle{};

    DebugInfo debug_info;

    void cpu_state_changed(Constants8008::CpuState old_state, Constants8008::CpuState state,
                           Scheduling::counter_type time);
    void cpu_sync_changed(Edge edge);
    void on_ready(Edge edge);
    void on_phase_2(Edge edge);
    void apply_signal_on_bus(const Constants8008::CpuState& state, unsigned long time);

    void connect_to_pluribus();
    void connect_to_cpu();
    void connect_to_clock();
    void connect_to_rtc();
};

#endif //MICRALN_PROCESSORCARD_H
