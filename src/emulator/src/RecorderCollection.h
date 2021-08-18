#ifndef MICRALN_RECORDERCOLLECTION_H
#define MICRALN_RECORDERCOLLECTION_H

#include "SignalRecorder.h"
#include "ValueRecorder.h"

#include <memory>
#include <string>
#include <unordered_map>

class RecorderCollection
{
public:
    SignalRecorder& create_and_get_signal_recorder(const std::string& signal_name,
                                                   double time_frame_in_s,
                                                   double projected_event_frequency);

    ValueRecorder& create_and_get_value_recorder(
            const std::string& signal_name, double time_frame_in_s,
            double projected_event_frequency, int8_t bus_width,
            ValueRecorder::OwnerTracking track_owners = ValueRecorder::TRACK_OWNERS);

    using container_type = std::unordered_map<std::string, std::shared_ptr<Recorder>>;

    [[nodiscard]] container_type::const_iterator begin() const;
    [[nodiscard]] container_type::const_iterator end() const;
    container_type::iterator begin();
    container_type::iterator end();

    const Recorder& get_by_name(const std::string& name) const;

    Scheduling::counter_type get_time_frame_as_counter() const;

private:
    container_type recorders;
    double general_time_frame_in_s{std::numeric_limits<double>::max()};
};

#endif //MICRALN_RECORDERCOLLECTION_H
