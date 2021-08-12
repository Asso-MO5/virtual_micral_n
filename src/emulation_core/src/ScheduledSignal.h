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

    std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables() override;

    void launch(Scheduling::counter_type start_time, Scheduling::counter_type duration,
                const Scheduling::change_schedule_cb& change_schedule);

    void step() override;

private:
    OwnedSignal& associated_signal;
    Scheduling::counter_type time_to_set_high{};
    Scheduling::counter_type time_to_set_low{};
};

#endif //MICRALN_SCHEDULEDSIGNAL_H
