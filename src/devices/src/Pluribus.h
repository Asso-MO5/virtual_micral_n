#ifndef MICRALN_PLURIBUS_H
#define MICRALN_PLURIBUS_H

#include <emulation_core/src/OwnedSignal.h>
#include <memory>

class DataBus;

class Pluribus
{
public:
    Pluribus();

    OwnedSignal vdd;
    OwnedSignal phase_2;
    OwnedSignal sync;
    OwnedSignal t2;
    OwnedSignal t3;
    OwnedSignal t3prime;
    OwnedSignal cc0;
    OwnedSignal cc1;
    OwnedSignal wait;
    OwnedSignal stop;

    std::shared_ptr<DataBus> data_bus_d0_7{};
};

#endif //MICRALN_PLURIBUS_H
