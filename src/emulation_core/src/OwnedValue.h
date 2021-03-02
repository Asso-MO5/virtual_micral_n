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

    OwnedValue() = default;
    explicit OwnedValue(ValueType start_value) : current_value{start_value} {}

    [[nodiscard]] ValueType get_state() const { return current_value; };
    ValueType operator*() const { return current_value; }
    [[nodiscard]] counter_type get_latest_change_time() const { return latest_change_time; }

    void request(void* requested_id)
    {
        if (owner_id != nullptr && requested_id != owner_id)
        {
            throw owned_value_error{"Cannot request, the value is already owned."};
        }
        owner_id = requested_id;
    }

    void release(void* release_id)
    {
        if (owner_id != nullptr && release_id != owner_id)
        {
            throw owned_value_error{"Cannot release when the value is not owned."};
        }
        owner_id = nullptr;
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

private:
    void* owner_id{};
    ValueType current_value{};
    counter_type latest_change_time{Scheduling::unscheduled()};
    std::vector<callback_type> callbacks;

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
};

#endif //MICRALN_OWNEDVALUE_H
