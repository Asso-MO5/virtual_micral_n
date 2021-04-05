#ifndef MICRALN_CONSOLECARD_H
#define MICRALN_CONSOLECARD_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>
#include <memory>

class Pluribus;

class ConsoleCard : public SchedulableImpl
{
public:
    explicit ConsoleCard(std::shared_ptr<Pluribus> pluribus);
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

        bool is_running;
        bool is_waiting;
        bool is_stopped;

        bool is_op_cycle;
        bool is_read_cycle;
        bool is_io_cycle;
        bool is_write_cycle;

        //uint8_t instruction;
        uint8_t data;
        uint16_t address;
    };

    enum StartMode
    {
        Automatic,
        Manual,
    };

    [[nodiscard]] Status get_status() const;

    void press_automatic();
    void press_stepping();
    void press_trap();

    void press_instruction();
    void press_cycle();

private:
    std::shared_ptr<Pluribus> pluribus;
    Status status;

    void on_phase_2(Edge edge);
    void on_t3(Edge edge);
    void on_sync(Edge edge);
};

#endif //MICRALN_CONSOLECARD_H
