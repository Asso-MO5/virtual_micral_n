#ifndef MICRALN_SCHEDULEDSIGNAL_H
#define MICRALN_SCHEDULEDSIGNAL_H

#include "Schedulable.h"
#include "Scheduling.h"

class OwnedSignal;

//
// Note: it's up to the user to warn the scheduler about the call to launch.
// TODO: in a future system where the Scheduler will detect scheduled change, the note will be outdated
class ScheduledSignal : public SchedulableImpl
{
public:
    explicit ScheduledSignal(OwnedSignal& signal);
    ~ScheduledSignal() override;

    void launch(unsigned long delay, Scheduling::counter_type duration);

    void step() override;

private:
    OwnedSignal& associated_signal;
    Scheduling::counter_type time_to_set_high{};
    Scheduling::counter_type time_to_set_low{};
};

#endif //MICRALN_SCHEDULEDSIGNAL_H
