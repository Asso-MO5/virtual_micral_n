#include "ScheduledSignal.h"

#include "OwnedSignal.h"

ScheduledSignal::ScheduledSignal(OwnedSignal& signal) : associated_signal(signal)
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

void ScheduledSignal::launch(Scheduling::counter_type delay, Scheduling::counter_type duration,
                             Scheduling::change_schedule_cb change_schedule_cb)
{
    time_to_set_high = delay;
    time_to_set_low = delay + duration;
    set_next_activation_time(time_to_set_high);
    // scheduler.change_schedule(get_id());
}
