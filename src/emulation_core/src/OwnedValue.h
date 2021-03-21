#ifndef MICRALN_OWNEDVALUE_H
#define MICRALN_OWNEDVALUE_H

#include "Scheduling.h"

#include <devices/src/MemoryCard.h>
#include <exception>
#include <functional>
#include <string>

struct owned_value_error : public std::exception
{
    explicit owned_value_error(const char* message) : msg(message) {}
    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

protected:
    std::string msg;
};

template<typename ValueType>
class OwnedValue
{
public:
    using counter_type = Scheduling::counter_type;
    using callback_type = std::function<void(ValueType, ValueType, counter_type)>;
    using callback_type_for_owner = std::function<void(void*, void*, counter_type)>;

    OwnedValue() = default;
    explicit OwnedValue(ValueType start_value) : current_value{start_value} {}

    [[nodiscard]] ValueType get_value() const { return current_value; };
    ValueType operator*() const { return current_value; }
    [[nodiscard]] counter_type get_latest_change_time() const { return latest_change_time; }

    void request(void* requested_id, counter_type time)
    {
        if (owner_id != nullptr && requested_id != owner_id)
        {
            throw owned_value_error{"Cannot request, the value is already owned."};
        }
        set_owner_and_broadcast(requested_id, time);
    }

    void release(void* release_id, counter_type time)
    {
        if (owner_id != nullptr && release_id != owner_id)
        {
            throw owned_value_error{"Cannot release when the value is not owned."};
        }
        set_owner_and_broadcast(nullptr, time);
    }

    void set(ValueType new_value, counter_type time, void* set_id)
    {
        if (owner_id != set_id)
        {
            throw owned_value_error{"Cannot set value when not owned."};
        }

        set_and_broadcast(new_value, time);
    }

    void subscribe(const callback_type& callback) { callbacks.push_back(callback); }
    void subscribe_to_owner(const callback_type_for_owner& callback)
    {
        callbacks_for_owner_change.push_back(callback);
    }

private:
    void* owner_id{};
    ValueType current_value{};
    counter_type latest_change_time{Scheduling::unscheduled()};
    std::vector<callback_type> callbacks;
    std::vector<callback_type_for_owner> callbacks_for_owner_change;

    void set_and_broadcast(ValueType new_value, counter_type time)
    {
        auto previous_state = current_value;

        if (previous_state != new_value)
        {
            current_value = new_value;
            latest_change_time = time;

            for (auto& callback : callbacks)
            {
                callback(previous_state, new_value, time);
            }
        }
    }

    void set_owner_and_broadcast(void* new_owner, counter_type time)
    {
        void* previous_owner = owner_id;
        owner_id = new_owner;

        if (previous_owner != new_owner)
        {
            for (auto& callback : callbacks_for_owner_change)
            {
                callback(previous_owner, new_owner, time);
            }
        }
    }
};

#endif //MICRALN_OWNEDVALUE_H
