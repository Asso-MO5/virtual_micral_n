#include "State.h"

void State::invert()
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
}
