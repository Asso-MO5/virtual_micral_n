#ifndef MICRALN_SCHEDULABLE_H
#define MICRALN_SCHEDULABLE_H

#include "Scheduling.h"

class Schedulable
{
public:
    virtual ~Schedulable() = default;
    virtual void step() = 0;
    [[nodiscard]] virtual Scheduling::counter_type get_next_activation_time() const = 0;
};

#endif //MICRALN_SCHEDULABLE_H
