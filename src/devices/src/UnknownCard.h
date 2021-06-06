#ifndef MICRALN_UNKNOWNCARD_H
#define MICRALN_UNKNOWNCARD_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>
#include <memory>

class IOCard;
class StackChannelCard;
class ScheduledSignal;

struct UnknownCardConfiguration
{
};

class UnknownCard : public SchedulableImpl
{
public:
    struct Config
    {
        Scheduling::change_schedule_cb change_schedule;
        std::shared_ptr<IOCard> io_card;
        std::shared_ptr<StackChannelCard> stack_channel;
        UnknownCardConfiguration configuration;
    };

    explicit UnknownCard(const Config& config);
    ~UnknownCard() override;

    std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables() override;

    void step() override;

private:
    Scheduling::change_schedule_cb change_schedule;
    std::shared_ptr<IOCard> io_card;
    std::shared_ptr<StackChannelCard> stack_channel;
    UnknownCardConfiguration configuration;

    struct Status
    {
        bool is_ready{};
        bool sending_to_channel{};
        uint8_t index_on_disk{};
    };

    struct NextSignalToLower
    {
        Scheduling::counter_type time_for_data_transfer{Scheduling::unscheduled()};
    };

    std::shared_ptr<ScheduledSignal> schedule_ack_2;

    Status status;
    NextSignalToLower next_signals_to_lower;

    void on_input_7(Edge edge);

    void on_transfer_enabled(Edge edge);
    void on_end_of_transfer(Edge edge);
};

#endif //MICRALN_UNKNOWNCARD_H
