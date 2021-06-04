#include "OwnedValue.h"

#include <gmock/gmock.h>

using namespace testing;

TEST(OwnedValue, is_initialized_at_unscheduled_time_with_a_default_value)
{
    OwnedValue<uint8_t> owned_value;

    ASSERT_THAT(owned_value.get_value(), Eq(0));
    ASSERT_THAT(owned_value.get_latest_change_time(), Eq(Scheduling::unscheduled()));
}

TEST(OwnedValue, can_be_initialized_with_a_value)
{
    OwnedValue<uint8_t> owned_value{20};

    ASSERT_THAT(owned_value.get_value(), Eq(20));
    ASSERT_THAT(owned_value.get_latest_change_time(), Eq(Scheduling::unscheduled()));
}

TEST(OwnedValue, can_be_deferenced)
{
    OwnedValue<uint8_t> owned_value{20};

    ASSERT_THAT(*owned_value, Eq(20));
}

TEST(OwnedValue, cannot_set_state_if_not_owned)
{
    OwnedValue<uint8_t> owned_value;
    uint16_t owner;

    ASSERT_THROW(owned_value.set(10, Scheduling::counter_type{1000}, static_cast<void*>(&owner)),
                 owned_value_error);
}

TEST(OwnedValue, cant_set_state_if_not_owned)
{
    OwnedValue<uint8_t> owned_value;
    uint16_t owner;

    owned_value.request(static_cast<void*>(&owner), 0);
    owned_value.set(10, Scheduling::counter_type{1000}, static_cast<void*>(&owner));

    ASSERT_THAT(owned_value.get_value(), Eq(10));
    ASSERT_THAT(owned_value.get_latest_change_time(), Eq(Scheduling::counter_type{1000}));
}

TEST(OwnedValue, cannot_be_requested_if_taken)
{
    OwnedValue<uint8_t> owned_value;
    uint16_t owner;
    uint16_t other_owner;

    owned_value.request(static_cast<void*>(&owner), 0);

    ASSERT_THROW(owned_value.request(static_cast<void*>(&other_owner), 0), owned_value_error);
}

TEST(OwnedValue, can_be_released)
{
    OwnedValue<uint8_t> owned_value;
    uint16_t owner;

    owned_value.request(static_cast<void*>(&owner), 0);
    owned_value.release(static_cast<void*>(&owner), 0);

    ASSERT_THROW(owned_value.set(10, Scheduling::counter_type{1000}, static_cast<void*>(&owner)),
                 owned_value_error);
}


TEST(OwnedValue, can_be_subscribed_to_for_value_change)
{
    using OwnedValueType = OwnedValue<uint8_t>;
    OwnedValueType owned_value;
    uint16_t owner;

    uint8_t received_old_value{};
    uint8_t received_new_value{};
    OwnedValueType::counter_type received_time{};

    owned_value.subscribe(
            [&](uint8_t old_value, uint8_t new_value, OwnedValueType::counter_type time) {
                received_new_value = new_value;
                received_old_value = old_value;
                received_time = time;
            });

    owned_value.request(static_cast<void*>(&owner), 0);
    owned_value.set(20, Scheduling::counter_type{2000}, static_cast<void*>(&owner));

    ASSERT_THAT(received_old_value, Eq(0));
    ASSERT_THAT(received_new_value, Eq(20));
    ASSERT_THAT(received_time, Eq(Scheduling::counter_type{2000}));
}

TEST(OwnedValue, can_be_subscribed_to_for_owner_change)
{
    using OwnedValueType = OwnedValue<uint8_t>;
    OwnedValueType owned_value;
    uint16_t owner;

    void* received_old_owner{};
    void* received_new_owner{};
    OwnedValueType::counter_type received_time{};

    owned_value.subscribe_to_owner(
            [&](void* old_value, void* new_value, OwnedValueType::counter_type time) {
              received_old_owner = old_value;
              received_new_owner = new_value;
              received_time = time;
            });

    owned_value.request(static_cast<void*>(&owner), Scheduling::counter_type{2000});

    void* saved_new_owner{received_new_owner};

    ASSERT_THAT(received_old_owner, IsNull());
    ASSERT_THAT(received_new_owner, Ne(nullptr));
    ASSERT_THAT(received_time, Eq(Scheduling::counter_type{2000}));

    owned_value.release(static_cast<void*>(&owner), Scheduling::counter_type{3000});

    ASSERT_THAT(received_old_owner, Eq(saved_new_owner));
    ASSERT_THAT(received_new_owner, IsNull());
    ASSERT_THAT(received_time, Eq(Scheduling::counter_type{3000}));


}
