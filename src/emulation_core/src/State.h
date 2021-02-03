#ifndef MICRALN_STATE_H
#define MICRALN_STATE_H

#include "Schedulable.h"
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

    constexpr explicit operator Type () const { return value; };

private:
    Type value;
    Scheduling::counter_type last_change_time;
};

#endif //MICRALN_STATE_H
