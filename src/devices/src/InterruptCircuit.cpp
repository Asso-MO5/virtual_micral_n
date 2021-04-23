#include "InterruptCircuit.h"

#include <emulation_core/src/OwnedSignal.h>

InterruptCircuit::InterruptCircuit(OwnedSignal& request_line, OwnedSignal& ack_line)
    : request_line{request_line}, ack_line{ack_line}
{
    connect();
}

void InterruptCircuit::connect()
{
    request_line.request(this);
    ack_line.subscribe([this](Edge edge) {
        if (is_rising(edge))
        {
            request_line.set(State::LOW, edge.time(), this);
        }
    });
}

void InterruptCircuit::trigger(Scheduling::counter_type time)
{
    request_line.set(State::HIGH, time, this);
}
