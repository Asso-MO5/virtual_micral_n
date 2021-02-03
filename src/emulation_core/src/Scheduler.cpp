#include "Scheduler.h"
#include "Schedulable.h"

#include <algorithm>
#include <cassert>

Scheduling::counter_type Scheduler::get_counter() const { return counter; }

void Scheduler::add(const Scheduler::schedulable_ptr& schedulable)
{
    schedulable_pool.emplace_back(schedulable->get_next_activation_time(), schedulable);
    sort_everything();
}

void Scheduler::step()
{
    if (schedulable_pool.empty())
    {
        return;
    }
    auto& [time, schedulable] = schedulable_pool.front();

    counter = time;
    executed_time = time;

    schedulable->step();
    time = schedulable->get_next_activation_time();
    assert(time >= executed_time);

    sort_everything();
}

void Scheduler::change_schedule(schedulable_ptr schedulable)
{
    auto find_schedulable = std::find_if(begin(schedulable_pool), end(schedulable_pool),
                                         [&schedulable](const auto& element) {
                                             const auto& [time, s] = element;
                                             return s.get() == schedulable.get();
                                         });

    if (find_schedulable != end(schedulable_pool))
    {
        auto time = schedulable->get_next_activation_time();
        if (time < executed_time)
        {
            throw std::runtime_error("The scheduler cannot go back in time.");
        }

        std::get<0>(*find_schedulable) = time;
        sort_everything();
    }
}

void Scheduler::sort_everything()
{
    std::sort(begin(schedulable_pool), end(schedulable_pool),
              [](const auto& a, const auto& b) { return std::get<0>(a) < std::get<0>(b); });

    assert(std::get<0>(schedulable_pool.front()) >= executed_time);
}
