#include "SerialCard.h"

#include "Pluribus.h"

#include <emulation_core/src/ScheduledSignal.h>

using namespace std;

SerialCard::SerialCard(const Config& config)
    : change_schedule{config.change_schedule}, pluribus{config.pluribus},
      configuration{config.configuration}
{
    input_data.request(this, Scheduling::counter_type{0});
    combined_status.request(this, Scheduling::counter_type{0});
    combined_status.set(0x80, Scheduling::counter_type{0}, this);
    combined_status_changed.request(this);

    output_strobe_AS.subscribe([this](Edge edge) { on_output(edge); });
    input_strobe_VE.subscribe([this](Edge edge) { on_input_strobe(edge); });

    input_ready_PE.request(this);
    output_ready_PS.request(this);

    input_ready_PE.subscribe([this](Edge edge) { on_input_ready(edge); });
    output_ready_PS.subscribe([this](Edge edge) { on_output_ready(edge); });

    pluribus->sync.subscribe([this](Edge edge) { on_sync(edge); });

    set_next_activation_time(Scheduling::unscheduled());
}

void SerialCard::step() {}

void SerialCard::on_sync(Edge edge)
{
    // Choosing this signal to constantly update the IO Card. Probably to check...
    auto time = edge.time();
    if (is_rising(edge))
    {
        combined_status_changed.set(State::HIGH, time, this);

        if (is_low(input_ready_PE) && !input_queue.empty())
        {
            // TODO: Does it needs something about parity?
            const char next_char = input_queue.front();
            input_queue.erase(begin(input_queue));
            input_data.set(next_char, time, this);

            input_ready_PE.set(State::HIGH, time, this);
        }
    }
    else
    {
        combined_status_changed.set(State::LOW, time, this);
    }
}

void SerialCard::add_input(char character) { input_queue.push_back(character); }

void SerialCard::on_output(Edge edge)
{
    if (is_rising(edge))
    {
        auto time = edge.time();

        output_ready_PS.set(State::LOW, time, this);

        auto value = *output_data;

        if (configuration.on_output_character)
        {
            configuration.on_output_character(value);
        }

        // At the moment, the peripheral is considered always ready to
        // receive characters.
        output_ready_PS.set(State::HIGH, time, this);
    }
}

void SerialCard::on_input_strobe(Edge edge)
{
    if (is_rising(edge))
    {
        // When received, reset the available character (DAV)
        input_ready_PE.set(State::LOW, edge.time(), this);
    }
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

vector<std::shared_ptr<Schedulable>> SerialCard::get_sub_schedulables()
{
    return {};
}
