#ifndef MICRALN_SCHEDULER_H
#define MICRALN_SCHEDULER_H

#include "Scheduling.h"

#include <memory>
#include <vector>

class Schedulable;

/*
 * A Scheduler takes a series a Schedulable and will call their step() method when their
 * activation time is due.
 *
 * It's a SignalReceiver which can be signaled that a Schedulable changed its next activation
 * time through `change_schedule`.
 */
class Scheduler
{
public:
    using schedulable_ptr = std::shared_ptr<Schedulable>;

    void add(const schedulable_ptr& schedulable);
    void step();

    void change_schedule(Scheduling::schedulable_id schedulable_id);

    [[nodiscard]] Scheduling::counter_type get_counter() const;

private:
    Scheduling::counter_type counter = 0;
    Scheduling::counter_type executed_time = 0;
    std::vector<std::tuple<Scheduling::counter_type, Scheduling::schedulable_id, schedulable_ptr>>
            schedulable_pool{};

    // The rough current number of actually scheduled Schedulables is kept, to allow
    // a very limited sub-sort.
    std::int32_t scheduled_count{};

    void sort_everything();
};

#endif //MICRALN_SCHEDULER_H
