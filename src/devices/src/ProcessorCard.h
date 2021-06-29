#ifndef MICRALN_PROCESSORCARD_H
#define MICRALN_PROCESSORCARD_H

#include "GeneralAddressRegister.h"

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/OwnedSignal.h>
#include <emulation_core/src/Schedulable.h>
#include <i8008/src/Constants8008.h>

#include <memory>

class AutomaticStart;
class CPU8008;
class Clock;
class DoubleClock;
class InterruptCircuit;
class InterruptController;
class Pluribus;

class ProcessorCard : public SchedulableImpl
{
public:
    struct Config
    {
        Scheduling::change_schedule_cb change_schedule;
        std::shared_ptr<Pluribus> pluribus;
    };

    explicit ProcessorCard(Config config);
    ~ProcessorCard() override;

    void step() override;

    [[nodiscard]] const CPU8008& get_cpu() const;

    std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables() override;

    DoubleClock& get_clock();
    Clock& get_rtc();

    struct DebugData
    {
        Scheduling::counter_type clock_pulse{};
        bool watchdog_on{};
    };

    void install_debug_info();
    [[nodiscard]] const DebugData& get_debug_data() const;

private:
    // Received as parameters
    std::shared_ptr<Pluribus> pluribus;
    Scheduling::change_schedule_cb change_schedule;

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

    DebugData debug_info;

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
