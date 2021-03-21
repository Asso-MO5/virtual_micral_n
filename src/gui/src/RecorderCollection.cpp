#include "RecorderCollection.h"

SignalRecorder& RecorderCollection::create_and_get(const std::string& signal_name,
                                                   double time_frame_in_s,
                                                   double projected_event_frequency)
{
    assert((recorders.find(signal_name) == std::end(recorders)) && "Signal already created");

    auto number_of_events =
            static_cast<size_t>(std::max(3.0, time_frame_in_s * projected_event_frequency));
    general_time_frame_in_s = std::min(general_time_frame_in_s, time_frame_in_s);

    auto new_signal_recorder = std::make_shared<SignalRecorder>(number_of_events);

    auto insertion_result = recorders.insert({signal_name, new_signal_recorder});
    assert(insertion_result.second && "Failed to create the recorder");

    return *new_signal_recorder;
}

RecorderCollection::container_type::const_iterator RecorderCollection::begin() const
{
    return std::cbegin(recorders);
}

RecorderCollection::container_type::const_iterator RecorderCollection::end() const
{
    return std::cend(recorders);
}

RecorderCollection::container_type::iterator RecorderCollection::begin()
{
    return std::begin(recorders);
}

RecorderCollection::container_type::iterator RecorderCollection::end()
{
    return std::end(recorders);
}

Scheduling::counter_type RecorderCollection::get_time_frame_as_counter() const
{
    return general_time_frame_in_s * 1'000'000'000;
}
