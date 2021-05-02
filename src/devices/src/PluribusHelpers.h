#ifndef MICRALN_PLURIBUSHELPERS_H
#define MICRALN_PLURIBUSHELPERS_H

#include <emulation_core/src/State.h>
#include <mcs8/src/Constants8008.h>

Constants8008::CycleControl cycle_control_from_cc(State cc0, State cc1);

#endif //MICRALN_PLURIBUSHELPERS_H
