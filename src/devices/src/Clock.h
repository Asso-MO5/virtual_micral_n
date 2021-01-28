#ifndef MICRALN_CLOCK_H
#define MICRALN_CLOCK_H

#include "Frequency.h"
#include "State.h"

#include "emulation_core/src/Schedulable.h"

class Clock : Schedulable
{
public:
    explicit Clock(Frequency frequency);

    void step() override;
    [[nodiscard]] Scheduling::counter_type get_next_activation_time() const override;
    [[nodiscard]] State get_state() const;

private:
    Frequency frequency;
    uint64_t next_activation_time = 0;
    State state = State::LOW;
};

#endif //MICRALN_CLOCK_H
