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
    bool scheduled_changed = false;
    action.schedule([&called](Scheduling::counter_type time) { called = true; },
                    Scheduling::counter_type{100},
                    [&scheduled_changed](Scheduling::schedulable_id) { scheduled_changed = true; });

    ASSERT_THAT(called, IsFalse());
    ASSERT_THAT(scheduled_changed, IsTrue());
}

TEST(ScheduledAction, sets_the_next_activation_time_when_scheduled)
{
    ScheduledAction action;

    action.schedule([](Scheduling::counter_type) {}, Scheduling::counter_type{100},
                    [](Scheduling::schedulable_id) {});

    ASSERT_THAT(action.get_next_activation_time(), Scheduling::counter_type{100});
}

TEST(ScheduledAction, does_not_accept_schedule_when_not_already_called)
{
    ScheduledAction action;

    action.schedule([](Scheduling::counter_type) {}, Scheduling::counter_type{100},
                    [](Scheduling::schedulable_id) {});
    ASSERT_THROW(action.schedule([](Scheduling::counter_type) {}, Scheduling::counter_type{200},
                                 [](Scheduling::schedulable_id) {}),
                 already_scheduled_error);
}

TEST(ScheduledAction, calls_the_function_when_stepped)
{
    ScheduledAction action;

    Scheduling::counter_type called_time{};
    action.schedule([&called_time](Scheduling::counter_type time) { called_time = time; },
                    Scheduling::counter_type{300}, [](Scheduling::schedulable_id) {});
    action.step();

    ASSERT_THAT(called_time, Scheduling::counter_type{300});
    ASSERT_THAT(action.get_next_activation_time(), Eq(Scheduling::unscheduled()));
}
