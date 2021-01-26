#include "Scheduler.h"
#include "Schedulable.h"

#include <algorithm>

Scheduling::counter_type Scheduler::get_counter() const
{
    return counter;
}

void Scheduler::add(const Scheduler::schedulable_ptr& schedulable)
{
    schedulable_pool.emplace_back(schedulable->get_next_activation_time(), schedulable);
    std::sort(begin(schedulable_pool), end(schedulable_pool),
              [](const auto& a, const auto& b) { return std::get<0>(a) < std::get<0>(b); });
}

void Scheduler::step()
{
    if (schedulable_pool.empty())
    {
        return;
    }
    auto& [time, schedulable] = schedulable_pool.front();
    counter = time;

    schedulable->step();
    time = schedulable->get_next_activation_time();

    std::sort(begin(schedulable_pool), end(schedulable_pool),
              [](const auto& a, const auto& b) { return std::get<0>(a) < std::get<0>(b); });
}
