#ifndef MICRALN_RECORDERCOLLECTION_H
#define MICRALN_RECORDERCOLLECTION_H

#include <gui/src/lib/SignalRecorder.h>
#include <string>

class RecorderCollection
{
public:
    SignalRecorder& create_and_get(const std::string& signal_name, double time_frame_in_s,
                                   double projected_event_frequency);

    using container_type = std::unordered_map<std::string, SignalRecorder>;

    container_type::const_iterator begin() const;
    container_type::const_iterator end() const;
    container_type::iterator begin() ;
    container_type::iterator end() ;

    Scheduling::counter_type get_time_frame_as_counter() const;

private:
    container_type recorders;
    double general_time_frame_in_s{std::numeric_limits<double>::max()};
};

#endif //MICRALN_RECORDERCOLLECTION_H
