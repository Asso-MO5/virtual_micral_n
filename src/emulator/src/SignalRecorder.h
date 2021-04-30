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
    virtual ~SignalRecorder() = default;

    void add(Edge edge);

    [[nodiscard]] std::size_t size() const override;
    [[nodiscard]] const double* time_series() const override;
    [[nodiscard]] const double* data_series() const override;

    [[nodiscard]] size_t owner_size() const override;
    [[nodiscard]] const double* owner_time_series() const override;
    [[nodiscard]] const std::uint32_t* owner_data_series() const override;

    uint8_t bus_width() const override;

private:
    std::vector<double> time_values;
    std::vector<double> state_values;
};

#endif //MICRALN_SIGNALRECORDER_H
