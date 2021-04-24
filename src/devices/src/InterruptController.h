#ifndef MICRALN_INTERRUPTCONTROLLER_H
#define MICRALN_INTERRUPTCONTROLLER_H

#include "Constants8008.h"

#include <functional>
#include <memory>

class CPU8008;
class Edge;
class GeneralAddressRegister;
class OwnedSignal;
class Pluribus;

const size_t INTERRUPT_LEVEL_COUNT = 8;

class InterruptController
{
public:
    explicit InterruptController(std::shared_ptr<Pluribus> pluribus, std::shared_ptr<CPU8008> cpu,
                                 std::shared_ptr<GeneralAddressRegister> bus_address_decoder);

    [[nodiscard]] bool has_instruction_to_inject() const;
    [[nodiscard]] uint8_t get_instruction_to_inject() const;

    void reset_lowest_interrupt();
    void on_phase_1(const Edge& edge);

private:
    std::shared_ptr<Pluribus> pluribus;
    std::shared_ptr<CPU8008> cpu;
    std::shared_ptr<GeneralAddressRegister> bus_address_decoder;
    std::array<bool, INTERRUPT_LEVEL_COUNT> requested_interrupts{};
    std::array<OwnedSignal*, INTERRUPT_LEVEL_COUNT> pluribus_int_ack{};
    uint8_t enabled_interrupts_mask{};
    bool interruption_are_enabled{};
    bool pending_out_17{};

    bool applying_interrupt{};
    uint8_t instruction_protection{};

    void request_signals();
    void connect_values();

    void on_t3_prime(Edge edge);
    void on_signal(Edge edge);

    void read_required_int_from_bus(OwnedSignal& signal, uint8_t level);
    [[nodiscard]] std::array<bool, INTERRUPT_LEVEL_COUNT> get_masked_requested() const;
    [[nodiscard]] bool has_a_requested_interrupt() const;
    [[nodiscard]] uint8_t lowest_level_interrupt() const;
    void reset_interrupt(uint8_t interrupt_level);

    void cpu_state_changed(Constants8008::CpuState old_state, Constants8008::CpuState new_state,
                           unsigned long time);

    // Instruction protection after an interruption routine
    void start_instruction_protection();
    void update_instruction_protection();
    [[nodiscard]] bool is_instruction_protected() const;
};

#endif //MICRALN_INTERRUPTCONTROLLER_H
