#include "ScheduledAction.h"

ScheduledAction::ScheduledAction() { set_next_activation_time(Scheduling::unscheduled()); }

void ScheduledAction::step()
{
    associated_function(get_next_activation_time());
    set_next_activation_time(Scheduling::unscheduled());
}

std::vector<std::shared_ptr<Schedulable>> ScheduledAction::get_sub_schedulables()
{
    return std::vector<std::shared_ptr<Schedulable>>();
}

void ScheduledAction::schedule(function_type function, Scheduling::counter_type time)
{
    if (get_next_activation_time() == Scheduling::unscheduled())
    {
        associated_function = function;
        set_next_activation_time(time);
    }
    else
    {
        throw already_scheduled_error{};
    }
}
