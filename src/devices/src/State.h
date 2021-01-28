#ifndef MICRALN_STATE_H
#define MICRALN_STATE_H

#include <cstdint>

struct State
{
    using Type = uint8_t;
    enum : Type
    {
        LOW,
        HIGH
    };

    void invert();

    State() = default;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
    constexpr State(Type value) : value(value) {}
#pragma clang diagnostic pop

    constexpr bool operator==(State other) const { return value == other.value; }
    constexpr bool operator!=(State other) const { return value != other.value; }

private:
    Type value;
};

#endif //MICRALN_STATE_H
