#ifndef MICRALN_SIGNALRECORDER_H
#define MICRALN_SIGNALRECORDER_H

#include "Recorder.h"

#include <cstdint>
#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Scheduling.h>
#include <vector>

class SignalRecorder : public Recorder
{
public:
    explicit SignalRecorder(std::size_t size);

    [[nodiscard]] std::size_t size() const override;
    [[nodiscard]] const double* time_series() const override;
    [[nodiscard]] const double* data_series() const override;

    void add(Edge edge);

private:
    std::vector<double> time_values;
    std::vector<double> state_values;
};

#endif //MICRALN_SIGNALRECORDER_H
