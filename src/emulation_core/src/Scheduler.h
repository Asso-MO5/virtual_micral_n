#ifndef MICRALN_SCHEDULER_H
#define MICRALN_SCHEDULER_H

#include "Scheduling.h"

#include <memory>
#include <vector>

class Schedulable;

class Scheduler
{
public:
    using schedulable_ptr = std::shared_ptr<Schedulable>;

    void add(const schedulable_ptr& schedulable);
    void step();

    [[nodiscard]] Scheduling::counter_type get_counter() const;

private:
    Scheduling::counter_type counter = 0;
    std::vector<std::tuple<Scheduling::counter_type, schedulable_ptr>> schedulable_pool{};
};

#endif //MICRALN_SCHEDULER_H
