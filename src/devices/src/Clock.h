#ifndef MICRALN_CLOCK_H
#define MICRALN_CLOCK_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Frequency.h>
#include <emulation_core/src/Schedulable.h>
#include <emulation_core/src/State.h>

#include <functional>
#include <utility>

class Clock : public SchedulableImpl
{
public:
    explicit Clock(Frequency frequency);

    void step() override;
    [[nodiscard]] State get_state() const;

    void register_trigger(std::function<void(Edge, Scheduling::counter_type)> callback);

private:
    Frequency frequency;
    std::function<void(Edge, Scheduling::counter_type)> edge_callback =
            [](Edge, Scheduling::counter_type) {};
    State state = State::LOW;
};

#endif //MICRALN_CLOCK_H
