#ifndef MICRALN_EDGE_H
#define MICRALN_EDGE_H

#include "State.h"

#include <cstdint>

struct Edge
{
    using Type = uint8_t;
    enum : Type
    {
        NONE,
        RISING,
        FALLING
    };

    Edge() = default;
    Edge(State before, State after)
    {
        if (before == after)
        {
            value = NONE;
        }
        else if (before == State::LOW)
        {
            value = RISING;
        }
        else
        {
            value = FALLING;
        }
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
    constexpr Edge(Type value) : value(value) {}
#pragma clang diagnostic pop

    constexpr bool operator==(Edge other) const { return value == other.value; }
    constexpr bool operator!=(Edge other) const { return value != other.value; }

private:
    Type value{};
};

#endif //MICRALN_EDGE_H
