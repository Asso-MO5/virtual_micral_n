#ifndef MICRALN_VALUERECORDER_H
#define MICRALN_VALUERECORDER_H

#include "SignalRecorder.h"
#include <cstdint>

class ValueRecorder : public Recorder
{
public:
    explicit ValueRecorder(std::size_t size);

    void add(uint16_t value, Scheduling::counter_type time);
    void change_owner(void* owner, Scheduling::counter_type time);

    [[nodiscard]] std::size_t size() const override;
    [[nodiscard]] const double* time_series() const override;
    [[nodiscard]] const double* data_series() const override;

    [[nodiscard]] const double* owner_time_series() const;
    [[nodiscard]] const std::uint32_t* owner_series() const;

private:
    std::vector<double> time_values;
    std::vector<double> state_values;

    std::vector<double> owner_time_values;
    std::vector<uint32_t> owner_values;
};

#endif //MICRALN_VALUERECORDER_H
