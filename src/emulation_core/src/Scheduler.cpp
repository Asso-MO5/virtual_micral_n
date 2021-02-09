#include "Scheduler.h"
#include "Schedulable.h"

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace
{
    uint64_t schedulable_id_counter{};

}
Scheduling::counter_type Scheduler::get_counter() const { return counter; }

void Scheduler::add(const Scheduler::schedulable_ptr& schedulable)
{
    schedulable_pool.emplace_back(schedulable->get_next_activation_time(), schedulable_id_counter,
                                  schedulable);
    schedulable->set_id(schedulable_id_counter);
    schedulable_id_counter += 1;
    sort_everything();
}

void Scheduler::step()
{
    if (schedulable_pool.empty())
    {
        return;
    }
    auto& [time, id, schedulable] = schedulable_pool.front();

    counter = time;
    executed_time = time;

    schedulable->step();
    time = schedulable->get_next_activation_time();
    assert(time >= executed_time);

    sort_everything();
}

void Scheduler::change_schedule(Scheduling::schedulable_id schedulable)
{
    auto find_schedulable = std::find_if(begin(schedulable_pool), end(schedulable_pool),
                                         [&schedulable](const auto& element) {
                                             const auto& [time, id, s] = element;
                                             return id == schedulable;
                                         });

    if (find_schedulable != end(schedulable_pool))
    {
        auto& [time, id, s] = *find_schedulable;
        auto new_time = s->get_next_activation_time();
        if (new_time < executed_time)
        {
            throw std::runtime_error("The scheduler cannot go back in time.");
        }
        time = new_time;
        sort_everything();
    }
}

void Scheduler::sort_everything()
{
    std::sort(begin(schedulable_pool), end(schedulable_pool),
              [](const auto& a, const auto& b) { return std::get<0>(a) < std::get<0>(b); });

    assert(std::get<0>(schedulable_pool.front()) >= executed_time);
}
