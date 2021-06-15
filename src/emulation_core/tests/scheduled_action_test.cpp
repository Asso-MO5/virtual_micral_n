#include "ScheduledAction.h"

#include <gmock/gmock.h>

using namespace testing;

TEST(ScheduledAction, is_a_schedulable_created_unscheduled)
{
    ScheduledAction action;

    ASSERT_THAT(action.get_next_activation_time(), Eq(Scheduling::unscheduled()));
}

TEST(ScheduledAction, can_schedule_a_function_without_calling_it)
{
    ScheduledAction action;

    bool called = false;
    action.schedule([&called](Scheduling::counter_type time) { called = true; },
                    Scheduling::counter_type{100});

    ASSERT_THAT(called, IsFalse());
}

TEST(ScheduledAction, sets_the_next_activation_time_when_scheduled)
{
    ScheduledAction action;

    action.schedule([](Scheduling::counter_type) {}, Scheduling::counter_type{100});

    ASSERT_THAT(action.get_next_activation_time(), Scheduling::counter_type{100});
}

TEST(ScheduledAction, does_not_accept_schedule_when_not_already_called)
{
    ScheduledAction action;

    action.schedule([](Scheduling::counter_type) {}, Scheduling::counter_type{100});
    ASSERT_THROW(action.schedule([](Scheduling::counter_type) {}, Scheduling::counter_type{200}),
                 already_scheduled_error);
}

TEST(ScheduledAction, calls_the_function_when_stepped)
{
    ScheduledAction action;

    Scheduling::counter_type called_time{};
    action.schedule([&called_time](Scheduling::counter_type time) { called_time = time; },
                    Scheduling::counter_type{300});
    action.step();

    ASSERT_THAT(called_time, Scheduling::counter_type{300});
    ASSERT_THAT(action.get_next_activation_time(), Eq(Scheduling::unscheduled()));
}
