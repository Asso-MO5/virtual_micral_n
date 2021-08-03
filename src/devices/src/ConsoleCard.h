#ifndef MICRALN_CONSOLECARD_H
#define MICRALN_CONSOLECARD_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>
#include <memory>

class Pluribus;
class ScheduledAction;

class ConsoleCard : public SchedulableImpl
{
public:
    enum StartMode
    {
        Automatic,
        Manual,
    };

    enum RecordMode
    {
        DontRecord,
        Record,
    };

    ConsoleCard(std::shared_ptr<Pluribus> pluribus, Scheduling::change_schedule_cb change_schedule,
                StartMode start_mode, RecordMode record_mode);
    ~ConsoleCard() override = default;

    std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables() override;
    void step() override;

    enum StepMode : uint8_t
    {
        Instruction,
        Cycle,
    };

    struct Status
    {
        bool automatic{};
        bool stepping{};
        bool trap{};
        bool substitution{};

        bool is_waiting{};
        bool is_stopped{};

        bool is_op_cycle{};
        bool is_read_cycle{};
        bool is_io_cycle{};
        bool is_write_cycle{};

        uint16_t address{};
        uint8_t data{};

        StepMode step_mode{Instruction};
    };

    class StatusHistory
    {
    public:
        using StatusContainer = std::vector<Status>;

        explicit StatusHistory(size_t size);

        void push(const ConsoleCard::Status& status);
        void reset();

        [[nodiscard]] const StatusContainer& get_history() const;

    private:
        StatusContainer history;
    };

    // Used with the current MDx data bus management, which will have to change.
    bool release_data_bus{};

    [[nodiscard]] Status get_status() const;
    [[nodiscard]] const StatusHistory::StatusContainer& get_status_history() const;
    void reset_history();

    void press_automatic();
    void press_stepping();
    void press_trap();

    void press_instruction();
    void press_cycle();

    void press_av();
    void press_interrupt();
    void press_substitution();

    void set_switch_data(uint8_t data);
    void set_switch_address(uint16_t address);

    void set_start_mode(StartMode mode);

private:
    StartMode start_mode;
    std::shared_ptr<Pluribus> pluribus;
    std::shared_ptr<ScheduledAction> place_data_on_pluribus;
    Scheduling::change_schedule_cb change_schedule;

    Status status;
    StatusHistory status_history;

    uint16_t switch_address{};
    uint8_t switch_data{};
    bool pending_interrupt{};

    void set_reset_state();

    void on_vdd(Edge edge);
    void on_phase_2(Edge edge);
    void on_sync(Edge edge);

    void set_step_mode();
    void on_rzgi(Edge edge);
    void on_init(Edge edge);
    void on_sub(Edge edge);
};

#endif //MICRALN_CONSOLECARD_H
