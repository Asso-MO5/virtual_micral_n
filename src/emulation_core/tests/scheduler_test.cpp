#include "Schedulable.h"
#include "Scheduler.h"

#include "gmock/gmock.h"

using namespace ::testing;

class MockSchedulable : public Schedulable
{
public:
    MOCK_METHOD(void, step, (), (override));
    MOCK_METHOD(Scheduling::counter_type, get_next_activation_time, (), (const));

    void set_id(Scheduling::schedulable_id new_id) override { id = new_id; };
    Scheduling::schedulable_id get_id() const override { return id; };
    Scheduling::schedulable_id id{};
    std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables() override { return {}; }
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

TEST(Scheduler, cannot_go_back_in_time)
{
    Scheduler scheduler;
    auto schedulable_A = std::make_shared<NiceMock<MockSchedulable>>();
    auto schedulable_B = std::make_shared<NiceMock<MockSchedulable>>();

    {
        InSequence sequence;
        EXPECT_CALL(*schedulable_A, get_next_activation_time()).WillOnce(Return(10));
        EXPECT_CALL(*schedulable_A, get_next_activation_time()).WillOnce(Return(40));
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

    {
        // New expected sequence trying to go back in time (earlier than the previous step)
        InSequence sequence;

        EXPECT_CALL(*schedulable_B, get_next_activation_time()).WillOnce(Return(19));
    }
    // Signal the schedule changed.
    ASSERT_THROW(scheduler.change_schedule(schedulable_B->get_id()), std::runtime_error);
}
