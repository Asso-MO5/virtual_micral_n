#ifndef MICRALN_SCHEDULER_H
#define MICRALN_SCHEDULER_H

#include "Scheduling.h"

#include <memory>
#include <vector>

class Schedulable;

class Scheduler : public SignalReceiver
{
public:
    using schedulable_ptr = std::shared_ptr<Schedulable>;

    void add(const schedulable_ptr& schedulable);
    void step();

    void change_schedule(Scheduling::schedulable_id schedulable);

    [[nodiscard]] Scheduling::counter_type get_counter() const;

private:
    Scheduling::counter_type counter = 0;
    Scheduling::counter_type executed_time = 0;
    std::vector<std::tuple<Scheduling::counter_type, Scheduling::schedulable_id, schedulable_ptr>>
            schedulable_pool{};

    void sort_everything();
};

#endif //MICRALN_SCHEDULER_H
