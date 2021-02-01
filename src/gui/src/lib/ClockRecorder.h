#ifndef MICRALN_CLOCKRECORDER_H
#define MICRALN_CLOCKRECORDER_H

#include <emulation_core/src/Scheduling.h>
#include <emulation_core/src/Edge.h>

#include <cstdint>
#include <vector>

class ClockRecorder
{
public:
    explicit ClockRecorder(std::size_t size);

    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] const float* time_series() const;
    [[nodiscard]] const float* state_series() const;

    void add(Scheduling::counter_type time, Edge edge);

private:
    std::vector<float> time_values;
    std::vector<float> state_values;
};

#endif //MICRALN_CLOCKRECORDER_H
