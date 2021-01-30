#ifndef MICRALN_CLOCK_H
#define MICRALN_CLOCK_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Frequency.h>
#include <emulation_core/src/Schedulable.h>
#include <emulation_core/src/State.h>

#include <functional>
#include <utility>

class Clock : public Schedulable
{
public:
    explicit Clock(Frequency frequency);

    void step() override;
    [[nodiscard]] Scheduling::counter_type get_next_activation_time() const override;
    [[nodiscard]] State get_state() const;

    void register_trigger(std::function<void(Edge, Scheduling::counter_type)> callback);

private:
    Frequency frequency;
    uint64_t next_activation_time = 0;
    std::function<void(Edge, Scheduling::counter_type)> edge_callback =
            [](Edge, Scheduling::counter_type) {};
    State state = State::LOW;
};

#endif //MICRALN_CLOCK_H
