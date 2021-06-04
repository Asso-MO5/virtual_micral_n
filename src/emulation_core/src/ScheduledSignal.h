#ifndef MICRALN_SCHEDULEDSIGNAL_H
#define MICRALN_SCHEDULEDSIGNAL_H

#include "Schedulable.h"
#include "Scheduling.h"

class OwnedSignal;

class ScheduledSignal : public SchedulableImpl
{
public:
    explicit ScheduledSignal(OwnedSignal& signal);
    ~ScheduledSignal() override;

    void launch(unsigned long delay, Scheduling::counter_type duration, SignalReceiver& scheduler);

    void step() override;

private:
    OwnedSignal& associated_signal;
    Scheduling::counter_type time_to_set_high{};
    Scheduling::counter_type time_to_set_low{};
};

#endif //MICRALN_SCHEDULEDSIGNAL_H
