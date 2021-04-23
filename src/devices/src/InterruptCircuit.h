#ifndef MICRALN_INTERRUPTCIRCUIT_H
#define MICRALN_INTERRUPTCIRCUIT_H

#include <emulation_core/src/Scheduling.h>

class OwnedSignal;

class InterruptCircuit
{
public:
    InterruptCircuit(OwnedSignal& request_line, OwnedSignal& ack_line);
    void trigger(Scheduling::counter_type time);

private:
    void connect();

    OwnedSignal& request_line;
    OwnedSignal& ack_line;
};

#endif //MICRALN_INTERRUPTCIRCUIT_H
