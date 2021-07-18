#include "SerialCard.h"

#include "Pluribus.h"

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

    input_ready_PE.subscribe([this](Edge edge) { on_input_ready(edge); });
    output_ready_PS.subscribe([this](Edge edge) { on_output_ready(edge); });

    pluribus->sync.subscribe([this](Edge edge) { on_sync(edge); });

    set_next_activation_time(Scheduling::unscheduled());
}

void SerialCard::step() {}

std::vector<std::shared_ptr<Schedulable>> SerialCard::get_sub_schedulables() { return {}; }

void SerialCard::on_sync(Edge edge)
{
    // Choosing this signal to constantly update the IO Card. Probably to check...
    auto time = edge.time();
    if (is_rising(edge))
    {
        combined_status_changed.set(State::HIGH, time, this);
    }
    else
    {
        combined_status_changed.set(State::LOW, time, this);
    }
}

void SerialCard::add_input(char character) { input_queue.push_back(character); }

void SerialCard::on_output(uint8_t value, Scheduling::counter_type time)
{
    output_ready_PS.set(State::LOW, time, this);

    auto output = static_cast<char>(value & 0x7f);

    if (configuration.on_output_character)
    {
        configuration.on_output_character(output);
    }

    // At the moment, the peripheral is considered always ready to
    // receive characters.
    output_ready_PS.set(State::HIGH, time, this);
}

void SerialCard::on_input_ready(Edge edge)
{
    auto status = combined_status.get_value() & 0xfe;
    status |= (is_rising(edge) ? 1 : 0);
    combined_status.set(status, edge.time(), this);
}

void SerialCard::on_output_ready(Edge edge)
{
    auto status = combined_status.get_value() & 0x7f;
    status |= (is_rising(edge) ? 1 : 0) << 7;
    combined_status.set(status, edge.time(), this);
}
