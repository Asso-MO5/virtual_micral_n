#ifndef MICRALN_SCHEDULING_H
#define MICRALN_SCHEDULING_H

#include <cstdint>
#include <functional>
#include <limits>

namespace Scheduling
{
    struct CounterType
    {
        using time_type = std::uint64_t;

        constexpr CounterType() : time(0) {}
        constexpr CounterType(std::uint64_t time) : time(time) {}

        constexpr CounterType operator+(CounterType other) const
        {
            return CounterType{time + other.time};
        }
        constexpr CounterType operator-(CounterType other) const
        {
            return CounterType{time - other.time};
        }
        constexpr CounterType operator+=(CounterType other) { return time += other.time; }

        // Cannot use starship operator at the moment because MingW doesn't support it yet.
        constexpr bool operator!=(CounterType other) const { return time != other.time; }
        constexpr bool operator==(CounterType other) const { return time == other.time; }
        constexpr bool operator<(CounterType other) const { return time < other.time; }
        constexpr bool operator>(CounterType other) const { return time > other.time; }
        constexpr bool operator>=(CounterType other) const { return time >= other.time; }

        constexpr time_type get() const { return time; }

        constexpr static CounterType unscheduled()
        {
            return CounterType{std::numeric_limits<time_type>::max()};
        }

    private:
        time_type time;
    };

    using counter_type = CounterType;
    using schedulable_id = uint32_t;
    inline constexpr counter_type unscheduled() { return CounterType::unscheduled(); }

    using change_schedule_cb = std::function<void(Scheduling::schedulable_id)>;
}

#endif //MICRALN_SCHEDULING_H
