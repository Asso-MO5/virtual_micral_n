#ifndef MICRALN_DOUBLECLOCK_H
#define MICRALN_DOUBLECLOCK_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Frequency.h>
#include <emulation_core/src/Schedulable.h>
#include <emulation_core/src/State.h>
#include <emulation_core/src/OwnedSignal.h>
#include <functional>

class DoubleClock : public SchedulableImpl
{
public:
    explicit DoubleClock(Frequency frequency);

    void step() override;
    [[nodiscard]] State get_phase_1_state() const;
    [[nodiscard]] State get_phase_2_state() const;

    void register_phase_1_trigger(const std::function<void(Edge)>& callback);
    void register_phase_2_trigger(const std::function<void(Edge)>& callback);

    OwnedSignal phase_1;
    OwnedSignal phase_2;

private:
    Frequency main_frequency;
    Scheduling::counter_type next_phase_1 = 0;
    uint8_t phase_count = 0;
};

#endif //MICRALN_DOUBLECLOCK_H
