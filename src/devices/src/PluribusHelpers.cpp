#include "PluribusHelpers.h"

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
