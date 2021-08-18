#ifndef MICRALN_SCHEDULEDACTION_H
#define MICRALN_SCHEDULEDACTION_H

#include "Schedulable.h"

#include <exception>
#include <vector>

struct already_scheduled_error : public std::exception
{
};

class ScheduledAction : public SchedulableImpl
{
public:
    using function_type = std::function<void(Scheduling::counter_type)>;
    ScheduledAction();
    void step() override;
    std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables() override;

    void schedule(function_type function, Scheduling::counter_type time,
                  const Scheduling::change_schedule_cb& change_schedule);

private:
    function_type associated_function;
};

#endif //MICRALN_SCHEDULEDACTION_H
