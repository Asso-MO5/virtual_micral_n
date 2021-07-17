#include "SerialCard.h"

#include <iostream>

using namespace std;

SerialCard::SerialCard(const Config& config)
    : change_schedule{config.change_schedule}, pluribus{config.pluribus},
      configuration{config.configuration}
{

    input_strobe_VE.request(this);
    input_data.request(this, Scheduling::counter_type{0});
    combined_status.request(this, Scheduling::counter_type{0});
    combined_status_changed.request(this);

    output_data.subscribe([this](uint8_t, uint8_t new_value, Scheduling::counter_type time) {
        on_output(new_value, time);
    });

    set_next_activation_time(Scheduling::counter_type{0});
}

void SerialCard::step()
{
    auto time = get_next_activation_time();

    // Force Output Ready at the moment
    combined_status.set(0x80, time, this);
    combined_status_changed.set(State::HIGH, time, this);

    set_next_activation_time(Scheduling::unscheduled());
}

std::vector<std::shared_ptr<Schedulable>> SerialCard::get_sub_schedulables() { return {}; }

void SerialCard::on_output(uint8_t value, Scheduling::counter_type time)
{
    auto output = static_cast<char>(value & 0x7f);

    if (output >= 32 || output == 10 || output == 13)
    {
        cout << output;
    }
    else
    {
        cout << '?';
    }

    set_next_activation_time(time + Scheduling::counter_type{100});
}
