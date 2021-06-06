#ifndef MICRALN_CLOCK_H
#define MICRALN_CLOCK_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Frequency.h>
#include <emulation_core/src/OwnedSignal.h>
#include <emulation_core/src/Schedulable.h>
#include <emulation_core/src/State.h>

#include <functional>
#include <utility>

class Clock : public SchedulableImpl
{
public:
    explicit Clock(Frequency frequency);

    std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables() override;

    void step() override;
    [[nodiscard]] State get_state() const;

    OwnedSignal phase;

private:
    Frequency frequency;
};

#endif //MICRALN_CLOCK_H
