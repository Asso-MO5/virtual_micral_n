#ifndef MICRALN_SIGNALRECORDER_H
#define MICRALN_SIGNALRECORDER_H

#include <emulation_core/src/Scheduling.h>
#include <emulation_core/src/Edge.h>

#include <cstdint>
#include <vector>

class SignalRecorder
{
public:
    explicit SignalRecorder(std::size_t size);

    [[nodiscard]] std::size_t size() const;
    [[nodiscard]] const double* time_series() const;
    [[nodiscard]] const double* state_series() const;

    void add(Edge edge);

private:
    std::vector<double> time_values;
    std::vector<double> state_values;
};

#endif //MICRALN_SIGNALRECORDER_H
