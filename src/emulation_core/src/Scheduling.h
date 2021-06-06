#ifndef MICRALN_SCHEDULING_H
#define MICRALN_SCHEDULING_H

#include <cstdint>
#include <functional>
#include <limits>

namespace Scheduling
{
    using counter_type = uint64_t;
    using schedulable_id = uint32_t;
    inline constexpr counter_type unscheduled() { return std::numeric_limits<counter_type>::max(); }

    using change_schedule_cb = std::function<void(Scheduling::schedulable_id)>;
}

#endif //MICRALN_SCHEDULING_H
