#ifndef MICRALN_EDGE_H
#define MICRALN_EDGE_H

#include "State.h"

#include <cassert>
#include <cstdint>

struct Edge
{
    using Type = uint8_t;
    enum class Front : Type
    {
        NONE,
        RISING,
        FALLING
    };

    Edge() = default;
    Edge(State before, State after, Scheduling::counter_type time) : timestamp(time)
    {
        if (before == after)
        {
            value = Front::NONE;
        }
        else if (before == State::LOW)
        {
            value = Front::RISING;
        }
        else
        {
            value = Front::FALLING;
        }
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
    constexpr Edge(Front value, Scheduling::counter_type time = Scheduling::counter_type{})
        : value(value), timestamp(time)
    {}
#pragma clang diagnostic pop

    [[nodiscard]] Scheduling::counter_type time() const { return timestamp; };

    constexpr bool operator==(Edge other) const { return value == other.value; }
    constexpr bool operator!=(Edge other) const { return value != other.value; }

    [[nodiscard]] State apply() const
    {
        switch (value)
        {
            case Front::RISING:
                return State{State::HIGH, time()};
            case Front::FALLING:
                return State{State::LOW, time()};
            default:
                assert(value != Front::NONE && "Applying a non edge to a State is an error.");
                return State{};
        }
    }

private:
    Front value{};
    Scheduling::counter_type timestamp{};
};

inline bool is_rising(const Edge& edge) { return edge == Edge::Front::RISING; }
inline bool is_falling(const Edge& edge) { return edge == Edge::Front::FALLING; }

#endif //MICRALN_EDGE_H
