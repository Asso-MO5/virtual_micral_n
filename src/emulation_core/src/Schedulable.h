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

class SchedulableImpl : public Schedulable
{
public:
    [[nodiscard]] Scheduling::counter_type get_next_activation_time() const override
    {
        return next_activation_time;
    }

protected:
    void set_next_activation_time(Scheduling::counter_type time) { next_activation_time = time; }

private:
    Scheduling::counter_type next_activation_time = 0;
};

#endif //MICRALN_SCHEDULABLE_H
