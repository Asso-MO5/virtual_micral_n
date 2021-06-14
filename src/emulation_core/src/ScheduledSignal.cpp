#include "ScheduledSignal.h"

#include "OwnedSignal.h"

ScheduledSignal::ScheduledSignal(OwnedSignal& signal) : associated_signal{signal}
{
    associated_signal.request(this);

    set_next_activation_time(Scheduling::unscheduled());
}

ScheduledSignal::~ScheduledSignal() { associated_signal.release(this); }

void ScheduledSignal::step()
{
    const auto time = get_next_activation_time();

    assert(time != Scheduling::unscheduled());

    if (time == time_to_set_high)
    {
        associated_signal.set(State::HIGH, time, this);
        set_next_activation_time(time_to_set_low);
    }
    else if (time == time_to_set_low)
    {
        associated_signal.set(State::LOW, time, this);
        set_next_activation_time(Scheduling::unscheduled());
    }
}

void ScheduledSignal::launch(Scheduling::counter_type start_time, Scheduling::counter_type duration,
                             const Scheduling::change_schedule_cb& change_schedule)
{
    assert(get_next_activation_time() == Scheduling::unscheduled() &&
           "New signal cannot be launched when the previous is not finished.");
    time_to_set_high = start_time;
    time_to_set_low = start_time + duration;
    set_next_activation_time(time_to_set_high);
    change_schedule(get_id());
}

std::vector<std::shared_ptr<Schedulable>> ScheduledSignal::get_sub_schedulables() { return {}; }
