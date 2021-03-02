#ifndef MICRALN_STATE_H
#define MICRALN_STATE_H

#include "Schedulable.h"

#include <algorithm>
#include <cstdint>

struct State
{
    using Type = uint8_t;
    enum : Type
    {
        LOW,
        HIGH
    };

    State() = default;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
    constexpr State(Type value) : value(value), last_change_time(0) {}
#pragma clang diagnostic pop

    constexpr State(Type value, Scheduling::counter_type time)
        : value(value), last_change_time(time)
    {}

    void invert(Scheduling::counter_type time);

    [[nodiscard]] Scheduling::counter_type last_change() const;

    constexpr bool operator==(State other) const { return value == other.value; }
    constexpr bool operator!=(State other) const { return value != other.value; }

    constexpr explicit operator Type() const { return value; }
    constexpr State operator&&(const State& other) const
    {
        bool state_as_bool = (value == HIGH);
        bool other_as_bool = (other.value == HIGH);

        return State{state_as_bool && other_as_bool,
                     std::max(last_change_time, other.last_change_time)};
    };

private:
    Type value;
    Scheduling::counter_type last_change_time;
};

inline bool is_high(const State& state) { return state == State::HIGH; }
inline bool is_low(const State& state) { return state == State::LOW; }

#endif //MICRALN_STATE_H
