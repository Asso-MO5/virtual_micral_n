#ifndef MICRALN_VALUERECORDER_H
#define MICRALN_VALUERECORDER_H

#include "SignalRecorder.h"

#include <cstdint>

class ValueRecorder : public Recorder
{
public:
    enum OwnerTracking
    {
        TRACK_OWNERS,
        DO_NOT_TRACK_OWNERS
    };

    explicit ValueRecorder(std::size_t size, uint8_t bus_width, OwnerTracking track_owners);

    void add(uint16_t value, Scheduling::counter_type time);
    void change_owner(void* owner, Scheduling::counter_type time);

    [[nodiscard]] std::size_t size() const override;
    [[nodiscard]] const double* time_series() const override;
    [[nodiscard]] const double* data_series() const override;

    [[nodiscard]] size_t owner_size() const override;
    [[nodiscard]] const double* owner_time_series() const override;
    [[nodiscard]] const std::uint32_t* owner_data_series() const override;

    [[nodiscard]] uint8_t bus_width() const override;

private:
    std::vector<double> time_values;
    std::vector<double> state_values;

    std::vector<double> owner_time_values;
    std::vector<uint32_t> owner_values;

    uint8_t data_bus_width;
};

#endif //MICRALN_VALUERECORDER_H
