#ifndef MICRALN_SCHEDULING_H
#define MICRALN_SCHEDULING_H

#include <cstdint>
#include <limits>

namespace Scheduling
{
    using counter_type = uint64_t;
    using schedulable_id = uint32_t;
    inline constexpr counter_type unscheduled() { return std::numeric_limits<counter_type>::max(); }
}

class SignalReceiver
{
public:
    virtual void change_schedule(Scheduling::schedulable_id) = 0;
};

#endif //MICRALN_SCHEDULING_H
