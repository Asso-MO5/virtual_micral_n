#ifndef MICRALN_UNKNOWNCARD_H
#define MICRALN_UNKNOWNCARD_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>

#include <emulation_core/src/Edge.h>
#include <memory>

class IOCard;
class StackChannelCard;

struct UnknownCardConfiguration
{
};

class UnknownCard : public SchedulableImpl
{
public:
    struct Config
    {
        SignalReceiver& scheduler;
        std::shared_ptr<IOCard> io_card;
        std::shared_ptr<StackChannelCard> stack_channel;
        UnknownCardConfiguration configuration;
    };

    explicit UnknownCard(const Config& config);
    ~UnknownCard() override;

    void step() override;

private:
    SignalReceiver& scheduler;
    std::shared_ptr<IOCard> io_card;
    std::shared_ptr<StackChannelCard> stack_channel;
    UnknownCardConfiguration configuration;

    struct Status
    {
        bool is_ready{};
        bool sending_to_channel{};
        bool end_of_send_cycle{};
        uint8_t bytes_to_send;
    };

    Status status;

    void on_input_4(Edge edge);
    void on_input_6(Edge edge);
    void on_input_7(Edge edge);
};

#endif //MICRALN_UNKNOWNCARD_H
