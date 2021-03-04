#ifndef MICRALN_PLURIBUSHELPERS_H
#define MICRALN_PLURIBUSHELPERS_H

#include "Constants8008.h"

#include <emulation_core/src/State.h>

Constants8008::CycleControl cycle_control_from_cc(State cc0, State cc1);

#endif //MICRALN_PLURIBUSHELPERS_H
