#ifndef MICRALN_SCHEDULING_H
#define MICRALN_SCHEDULING_H

#include <cstdint>
#include <limits>

namespace Scheduling
{
    using counter_type = uint64_t;
    inline counter_type unscheduled() { return std::numeric_limits<counter_type>::max(); }
}

#endif //MICRALN_SCHEDULING_H
