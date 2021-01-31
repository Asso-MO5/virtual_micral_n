#include "Schedulable.h"
#include "Scheduler.h"

#include "gmock/gmock.h"

using namespace ::testing;

class MockSchedulable : public Schedulable
{
public:
    MOCK_METHOD(void, step, (), (override));
    MOCK_METHOD(Scheduling::counter_type, get_next_activation_time, (), (const));
};

TEST(Scheduler, newly_instantiated_starts_with_counter_at_0)
{
    Scheduler scheduler;

    ASSERT_THAT(scheduler.get_counter(), Eq(0LL));
}

TEST(Scheduler, can_be_stepped_even_empty)
{
    Scheduler scheduler;
    EXPECT_NO_FATAL_FAILURE(scheduler.step());
}

TEST(Scheduler, calls_step_in_order_A_then_B)
{
    Scheduler scheduler;
    auto schedulable_A = std::make_shared<NiceMock<MockSchedulable>>();
    auto schedulable_B = std::make_shared<NiceMock<MockSchedulable>>();

    {
        InSequence sequence;
        EXPECT_CALL(*schedulable_A, get_next_activation_time()).WillOnce(Return(10));
        EXPECT_CALL(*schedulable_A, get_next_activation_time()).WillRepeatedly(Return(100));
    }
    {
        InSequence sequence;
        EXPECT_CALL(*schedulable_B, get_next_activation_time()).WillOnce(Return(20));
        EXPECT_CALL(*schedulable_B, get_next_activation_time()).WillRepeatedly(Return(100));
    }
    scheduler.add(schedulable_A);
    scheduler.add(schedulable_B);

    {
        InSequence sequence;
        EXPECT_CALL(*schedulable_A, step);
        EXPECT_CALL(*schedulable_B, step);
    }

    scheduler.step();
    scheduler.step();
}

TEST(Scheduler, calls_step_in_order_B_then_A)
{
    Scheduler scheduler;
    auto schedulable_A = std::make_shared<NiceMock<MockSchedulable>>();
    auto schedulable_B = std::make_shared<NiceMock<MockSchedulable>>();

    {
        InSequence sequence;
        EXPECT_CALL(*schedulable_A, get_next_activation_time()).WillOnce(Return(20));
        EXPECT_CALL(*schedulable_A, get_next_activation_time()).WillRepeatedly(Return(100));
    }
    {
        InSequence sequence;
        EXPECT_CALL(*schedulable_B, get_next_activation_time()).WillOnce(Return(10));
        EXPECT_CALL(*schedulable_B, get_next_activation_time()).WillRepeatedly(Return(100));
    }

    scheduler.add(schedulable_A);
    scheduler.add(schedulable_B);

    {
        InSequence sequence;
        EXPECT_CALL(*schedulable_B, step);
        EXPECT_CALL(*schedulable_A, step);
    }

    scheduler.step();
    scheduler.step();
}

TEST(Scheduler, calls_step_in_order_A_A_B_A_B)
{
    Scheduler scheduler;
    auto schedulable_A = std::make_shared<NiceMock<MockSchedulable>>();
    auto schedulable_B = std::make_shared<NiceMock<MockSchedulable>>();

    {
        InSequence sequence;

        for (const auto& value : {10, 11, 20})
        {
            EXPECT_CALL(*schedulable_A, get_next_activation_time()).WillOnce(Return(value));
        }
        EXPECT_CALL(*schedulable_A, get_next_activation_time()).WillRepeatedly(Return(100));
    }

    {
        InSequence sequence;
        for (const auto& value : {15, 30})
        {
            EXPECT_CALL(*schedulable_B, get_next_activation_time()).WillOnce(Return(value));
        }
        EXPECT_CALL(*schedulable_B, get_next_activation_time()).WillRepeatedly(Return(100));
    }

    scheduler.add(schedulable_A);
    scheduler.add(schedulable_B);

    {
        InSequence sequence;
        EXPECT_CALL(*schedulable_A, step);
        EXPECT_CALL(*schedulable_A, step);
        EXPECT_CALL(*schedulable_B, step);
        EXPECT_CALL(*schedulable_A, step);
        EXPECT_CALL(*schedulable_B, step);
    }

    scheduler.step();
    scheduler.step();
    scheduler.step();
    scheduler.step();
    scheduler.step();

    ASSERT_THAT(scheduler.get_counter(), Eq(30LL));
}
