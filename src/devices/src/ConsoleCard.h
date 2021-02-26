#ifndef MICRALN_CONSOLECARD_H
#define MICRALN_CONSOLECARD_H

#include <emulation_core/src/Schedulable.h>
#include <memory>

class Pluribus;

class ConsoleCard : public SchedulableImpl
{
public:
    explicit ConsoleCard(std::shared_ptr<Pluribus>  pluribus);
    ~ConsoleCard() override = default;

    void step() override;
    enum StepMode
    {
        None,
        Instruction,
        Cycle,
    };

    struct Status
    {
        bool automatic;
        bool stepping;
        bool trap;
        bool substitution;

        StepMode step_mode;

        bool is_running;
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

private:
    std::shared_ptr<Pluribus> pluribus;
    Status status;
};

#endif //MICRALN_CONSOLECARD_H
