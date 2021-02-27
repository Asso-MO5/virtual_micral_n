#include "Pluribus.h"

#include <emulation_core/src/DataBus.h>

Pluribus::Pluribus() { data_bus_d0_7 = std::make_shared<DataBus>();
}
