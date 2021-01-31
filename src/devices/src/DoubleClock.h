#ifndef MICRALN_DOUBLECLOCK_H
#define MICRALN_DOUBLECLOCK_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Frequency.h>
#include <emulation_core/src/Schedulable.h>
#include <emulation_core/src/State.h>

#include <functional>

class DoubleClock : public Schedulable
{
public:
    explicit DoubleClock(Frequency frequency);

    void step() override;
    [[nodiscard]] Scheduling::counter_type get_next_activation_time() const override;
    [[nodiscard]] State get_phase_1_state() const;
    [[nodiscard]] State get_phase_2_state() const;

    void register_phase_1_trigger(std::function<void(Edge, Scheduling::counter_type)> callback);
    void register_phase_2_trigger(std::function<void(Edge, Scheduling::counter_type)> callback);

private:
    Frequency main_frequency;
    Scheduling::counter_type next_activation_time = 0;
    Scheduling::counter_type next_phase_1 = 0;

    State phase_1 = State::LOW;
    State phase_2 = State::LOW;

    std::function<void(Edge, Scheduling::counter_type)> edge_1_callback =
            [](Edge, Scheduling::counter_type) {};
    std::function<void(Edge, Scheduling::counter_type)> edge_2_callback =
            [](Edge, Scheduling::counter_type) {};


    uint8_t phase_count = 0;
};

#endif //MICRALN_DOUBLECLOCK_H
