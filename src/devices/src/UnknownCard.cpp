#include "UnknownCard.h"
#include <iostream>

#include "IOCard.h"
#include "Pluribus.h"
#include "StackChannelCard.h"

#include <emulation_core/src/ScheduledSignal.h>

using namespace std;

namespace
{
    uint8_t disk_data[] = {
            'S', 'T', 'A', 'R', 'T', ' ', ' ', ' ', ' ', ' ', 'H', 'E',  'L',  'L', 'O', 'W', 'O',
            'R', 'L', 'D', 'H', 'E', 'L', 'L', 'O', 'W', 'O', 'R', 'L',  'D',  'H', 'E', 'L', 'L',
            'O', 'W', 'O', 'R', 'L', 'D', 'H', 'E', 'L', 'L', 'O', 'W',  'O',  'R', 'L', 'D', 'H',
            'E', 'L', 'L', 'O', 'W', 'O', 'R', 'L', 'D', 'H', 'E', 'L',  'L',  'O', 'W', 'O', 'R',
            'L', 'D', 'H', 'E', 'L', 'L', 'O', 'W', 'O', 'R', 'L', 'D',  'H',  'E', 'L', 'L', 'O',
            'W', 'O', 'R', 'L', 'D', 'H', 'E', 'L', 'L', 'O', 'W', 'O',  'R',  'L', 'D', 'H', 'E',
            'L', 'L', 'O', 'W', 'O', 'R', 'L', 'D', 'H', 'E', 'L', 'L',  'O',  'W', 'O', 'R', 'L',
            'D', 'H', 'E', 'L', 'L', 'O', 'W', 'O', 'R', 'L', 'D', 0x7c, 0x00,
    };
}

UnknownCard::UnknownCard(const Config& config)
    : change_schedule{config.change_schedule}, configuration{config.configuration}
{
    card_status.request(this, Scheduling::counter_type{0});
    receive_command.subscribe([this](Edge edge) { on_command(edge); });

    schedule_status_changed = std::make_shared<ScheduledSignal>(status_changed);

    schedule_available_data = std::make_shared<ScheduledSignal>(available_data);
    output_data.request(this, Scheduling::counter_type{0});

    start_data_transfer.subscribe([this](Edge edge) { on_transfer_enabled(edge); });
    stop_data_transfer.subscribe([this](Edge edge) { on_end_of_transfer(edge); });

    set_next_activation_time(Scheduling::unscheduled());
}

UnknownCard::~UnknownCard() = default;

void UnknownCard::step() {}

void UnknownCard::on_command(Edge edge)
{
    if (is_rising(edge))
    {
        // Bit 7:
        // Bit 6: I/O Ready (can be a short pulse) -> Wake up the unknown card
        // Bit 5: Next Chunk
        // Bit 4: First round of E
        // Bit 0-3: Counter

        const uint8_t data = *command;

        if (data & 0b01000000 || data & 0b00100000)
        {
            auto received_command = static_cast<std::uint16_t>(data >> 4);
            cout << "--> ";
            cout << "REQ (";
            cout << received_command << ") ";

            if (!status.is_ready)
            {
                status.is_ready = true;
                card_status.set(0b10000011, edge.time(), this);

                schedule_status_changed->launch(edge.time(), Scheduling::counter_type{100},
                                                change_schedule);
            }
            if (data & 0b00010000)
            {
                if (status.is_ready && !status.sending_to_channel)
                {
                    cout << "GO - ";
                    status.sending_to_channel = true;
                    status.index_on_disk = 0;

                    card_status.set(0b00000011, edge.time(), this);

                    schedule_status_changed->launch(edge.time(), Scheduling::counter_type{100},
                                                    change_schedule);
                }
                else
                {
                    cout << "ALREADY GOING - ";
                }
            }
            cout << "E: " << static_cast<uint32_t>(data & 0b1111);
            cout << endl;
        }
        else if (data == 0)
        {
            cout << "STOP - 00" << endl;
        }

        assert(((data & 0b10000000) == 0) &&
               "Signal not handled"); // TODO: temporary assert. This should be some sort of emulation error.
    }
}

void UnknownCard::on_transfer_enabled(Edge edge)
{
    if (is_rising(edge))
    {
        if (status.sending_to_channel)
        {
            const auto time = edge.time();
            const auto data_to_send = disk_data[status.index_on_disk];
            status.index_on_disk += 1;

            if (status.index_on_disk >= sizeof(disk_data))
            {
                status.sending_to_channel = false;
                status.is_ready = false;
            }
            else
            {
                output_data.set(data_to_send, time, this);
                schedule_available_data->launch(edge.time(), Scheduling::counter_type{100},
                                                change_schedule);
            }
        }
    }
}

void UnknownCard::on_end_of_transfer(Edge edge)
{
    if (is_rising(edge) && status.sending_to_channel)
    {
        status.sending_to_channel = false;
        status.is_ready = false;

        card_status.set(0b00000000, edge.time(), this);

        schedule_status_changed->launch(edge.time(), Scheduling::counter_type{100},
                                        change_schedule);
    }
}

std::vector<std::shared_ptr<Schedulable>> UnknownCard::get_sub_schedulables()
{
    return {schedule_status_changed, schedule_available_data};
}
