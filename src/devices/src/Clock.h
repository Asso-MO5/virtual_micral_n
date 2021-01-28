#ifndef MICRALN_CLOCK_H
#define MICRALN_CLOCK_H

#include "Edge.h"
#include "Frequency.h"
#include "State.h"

#include "emulation_core/src/Schedulable.h"

#include <functional>
#include <utility>

class Clock : Schedulable
{
public:
    explicit Clock(Frequency frequency);

    void step() override;
    [[nodiscard]] Scheduling::counter_type get_next_activation_time() const override;
    [[nodiscard]] State get_state() const;

    void register_trigger(std::function<void(Edge)> callback);

private:
    Frequency frequency;
    uint64_t next_activation_time = 0;
    std::function<void(Edge)> edge_callback = [](Edge){};
    State state = State::LOW;
};

#endif //MICRALN_CLOCK_H
