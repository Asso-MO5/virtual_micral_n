#ifndef MICRALN_PLURIBUS_H
#define MICRALN_PLURIBUS_H

#include <emulation_core/src/OwnedSignal.h>
#include <emulation_core/src/OwnedValue.h>
#include <memory>

class Pluribus
{
public:
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

    OwnedSignal mrst;
    OwnedSignal bi1;
    OwnedSignal bi2;
    OwnedSignal bi3;
    OwnedSignal bi4;
    OwnedSignal bi5;
    OwnedSignal bi6;
    OwnedSignal bi7;

    OwnedSignal rzgi;
    OwnedSignal aint1;
    OwnedSignal aint2;
    OwnedSignal aint3;
    OwnedSignal aint4;
    OwnedSignal aint5;
    OwnedSignal aint6;
    OwnedSignal aint7;

    OwnedSignal init;

    OwnedSignal ready_console;
    OwnedSignal ready;

    OwnedValue<uint8_t> data_bus_d0_7{};
    OwnedValue<uint8_t> data_bus_md0_7{};
    OwnedValue<uint16_t> address_bus_s0_s13{};
};

#endif //MICRALN_PLURIBUS_H
