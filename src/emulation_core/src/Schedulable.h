#ifndef MICRALN_SCHEDULABLE_H
#define MICRALN_SCHEDULABLE_H

#include "Scheduling.h"

#include <memory>

/*
 * A Schedulable is anything that can be scheduled by the Scheduler
 */
class Schedulable
{
public:
    virtual ~Schedulable() = default;
    virtual void step() = 0;
    virtual void set_id(Scheduling::schedulable_id) = 0;
    [[nodiscard]] virtual Scheduling::schedulable_id get_id() const = 0;
    [[nodiscard]] virtual Scheduling::counter_type get_next_activation_time() const = 0;
    [[nodiscard]] virtual std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables() = 0;
};

/*
 * SchedulableImpl is a facility with activation time and id implemented.
 */
class SchedulableImpl : public Schedulable
{
public:
    [[nodiscard]] Scheduling::counter_type get_next_activation_time() const override
    {
        return next_activation_time;
    }

protected:
    void set_next_activation_time(Scheduling::counter_type time) { next_activation_time = time; }
    void set_id(Scheduling::schedulable_id new_id) override { id = new_id; };
    [[nodiscard]] Scheduling::schedulable_id get_id() const override { return id; };

private:
    Scheduling::counter_type next_activation_time{};
    Scheduling::schedulable_id id{};
};

#endif //MICRALN_SCHEDULABLE_H
