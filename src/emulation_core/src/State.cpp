#include "State.h"

void State::invert(Scheduling::counter_type time)
{
    switch (value)
    {
        case LOW:
            value = HIGH;
            break;
        case HIGH:
            value = LOW;
            break;
    }
    last_change_time = time;
}

Scheduling::counter_type State::last_change() const { return last_change_time; }

