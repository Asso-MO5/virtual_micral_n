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
    assert(std::find_if(begin(schedulable_pool), end(schedulable_pool),
                        [&schedulable](const auto& element) {
                            const auto& [time, id, s] = element;
                            return s == schedulable;
                        }) == end(schedulable_pool) &&
           "Schedulable already in the pool.");
    schedulable_pool.emplace_back(schedulable->get_next_activation_time(), schedulable_id_counter,
                                  schedulable);
    schedulable->set_id(schedulable_id_counter);
    schedulable_id_counter += 1;

    for (auto& sub : schedulable->get_sub_schedulables())
    {
        add(sub);
    }

    if (schedulable->get_next_activation_time() != Scheduling::unscheduled())
    {
        // Force a full reschedule, as the added element is at the end.
        assert(schedulable_pool.size() < std::numeric_limits<int32_t>::max());
        scheduled_count = static_cast<int32_t>(schedulable_pool.size());
    }

    sort_everything();
}

void Scheduler::step()
{
    if (schedulable_pool.empty())
    {
        return;
    }

    sort_everything();

    auto& [time, id, schedulable] = schedulable_pool.front();

    counter = time;
    executed_time = time;

    // Get the raw pointer on the Schedulable to assert if it's still the same after
    // calling step()> If not, it means the pool was shuffled (probably sorted) while
    // stepping the devices, which is breaking the contract.
    Schedulable* raw_schedulable = schedulable.get();

    schedulable->step();

    assert(schedulable.get() == raw_schedulable &&
           "The pool probably has been sorted out while stepping.");
    time = schedulable->get_next_activation_time();

    assert(time >= executed_time);

    sort_everything();
}

void Scheduler::change_schedule(Scheduling::schedulable_id schedulable_id)
{
    auto find_schedulable = std::find_if(begin(schedulable_pool), end(schedulable_pool),
                                         [&schedulable_id](const auto& element) {
                                             const auto& [time, id, s] = element;
                                             return id == schedulable_id;
                                         });

    if (find_schedulable != end(schedulable_pool))
    {
        auto& [time, id, s] = *find_schedulable;
        auto previous_time = time;
        auto new_time = s->get_next_activation_time();
        if (new_time < executed_time)
        {
            throw std::runtime_error("The scheduler cannot go back in time.");
        }
        time = new_time;

        if (previous_time == Scheduling::unscheduled() && time != Scheduling::unscheduled())
        {
            // The Schedulable is Now scheduled, swap it with the first unscheduled and
            // add to count of actually scheduled.
            // In the other cases, nothing is done, the next sort will push the leftovers
            // after the scheduled elements, and the count will be adjusted.
            if (static_cast<std::size_t>(scheduled_count) < schedulable_pool.size())
            {
                std::swap(*(begin(schedulable_pool) + scheduled_count), *find_schedulable);
            }
            scheduled_count += 1;
        }
    }
}

void Scheduler::sort_everything()
{
    // Sort only the first elements, which are ensured to contain all the scheduled
    // elements (but can also contain unscheduled elements).
    std::sort(begin(schedulable_pool), begin(schedulable_pool) + scheduled_count,
              [](const auto& a, const auto& b) { return std::get<0>(a) < std::get<0>(b); });

    // Find the new number of actually scheduled elements.
    const auto& first_unscheduled =
            std::find_if(begin(schedulable_pool), end(schedulable_pool), [](const auto& element) {
                const auto& [time, id, s] = element;
                return time == Scheduling::unscheduled();
            });

    scheduled_count =
            static_cast<int32_t>(std::distance(begin(schedulable_pool), first_unscheduled));

    assert(std::get<0>(schedulable_pool.front()) >= executed_time);
}
