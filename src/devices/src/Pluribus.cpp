#include "Pluribus.h"

Constants8008::CycleControl cycle_control_from_cc(State cc0, State cc1)
{
    Constants8008::CycleControl cycleControl;
    if (is_low(cc0))
    {
        cycleControl = is_low(cc1) ? Constants8008::CycleControl::PCI
                                   : Constants8008::CycleControl::PCR;
    }
    else
    {
        cycleControl = is_low(cc1) ? Constants8008::CycleControl::PCC
                                   : Constants8008::CycleControl::PCW;
    }
    return cycleControl;
}

std::tuple<uint16_t, Constants8008::CycleControl> decode_address_on_bus(const Pluribus& pluribus)
{
    uint16_t address = *pluribus.address_bus_s0_s13 & 0x3fff;

    auto cc0 = *pluribus.cc0;
    auto cc1 = *pluribus.cc1;

    Constants8008::CycleControl cycleControl = cycle_control_from_cc(cc0, cc1);

    return std::make_tuple(address, cycleControl);
}
