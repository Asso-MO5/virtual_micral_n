#ifndef MICRALN_CONSOLECARD_H
#define MICRALN_CONSOLECARD_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>
#include <memory>

class Pluribus;

class ConsoleCard : public SchedulableImpl
{
public:
    enum StartMode
    {
        Automatic,
        Manual,
    };

    explicit ConsoleCard(std::shared_ptr<Pluribus> pluribus, StartMode start_mode);
    ~ConsoleCard() override = default;

    void step() override;
    enum StepMode
    {
        Instruction,
        Cycle,
    };

    struct Status
    {
        bool automatic;
        bool stepping;
        bool trap;
        bool substitution;

        StepMode step_mode{Instruction};

        bool is_waiting;
        bool is_stopped;

        bool is_op_cycle;
        bool is_read_cycle;
        bool is_io_cycle;
        bool is_write_cycle;

        uint8_t data;
        uint16_t address;
    };

    [[nodiscard]] Status get_status() const;

    void press_automatic();
    void press_stepping();
    void press_trap();

    void press_instruction();
    void press_cycle();

    void set_switch_data(uint8_t i);

    void set_switch_address(uint16_t i);

    void press_interrupt();

private:
    StartMode start_mode;
    std::shared_ptr<Pluribus> pluribus;
    Status status;
    uint16_t switch_address{};
    uint8_t switch_data{};
    bool pending_interrupt{};

    void on_vdd(Edge edge);
    void on_phase_2(Edge edge);
    void on_t3(Edge edge);
    void on_sync(Edge edge);

    void set_step_mode();
    void on_rzgi(Edge edge);
};

#endif //MICRALN_CONSOLECARD_H
