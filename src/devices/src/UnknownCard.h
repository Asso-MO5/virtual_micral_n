#ifndef MICRALN_UNKNOWNCARD_H
#define MICRALN_UNKNOWNCARD_H

#include <emulation_core/src/Schedulable.h>

#include <memory>

class IOCard;

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
        UnknownCardConfiguration configuration;
    };

    explicit UnknownCard(const Config& config);
    ~UnknownCard() override;

    void step() override;

private:
    SignalReceiver& scheduler;
    std::shared_ptr<IOCard> io_card;
    UnknownCardConfiguration configuration;

};

#endif //MICRALN_UNKNOWNCARD_H
