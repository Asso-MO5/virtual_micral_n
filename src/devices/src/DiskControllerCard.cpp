#include "DiskControllerCard.h"
#include <iostream>

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

DiskControllerCard::DiskControllerCard(const Config& config)
    : change_schedule{config.change_schedule}, configuration{config.configuration}
{
    card_status.request(this, Scheduling::counter_type{0});
    receive_command.subscribe([this](Edge edge) { on_command(edge); });

    schedule_status_changed = std::make_shared<ScheduledSignal>(status_changed);

    schedule_available_data = std::make_shared<ScheduledSignal>(available_data);
    output_data.request(this, Scheduling::counter_type{0});

    start_data_transfer.subscribe([this](Edge edge) { on_transfer_enabled(edge); });
    stop_data_transfer.subscribe([this](Edge edge) { on_end_of_transfer(edge); });

    activate.subscribe([this](Edge edge) { on_activate(edge); });

    direction.request(this);

    internal.step.request(this);
    internal.step.subscribe([this](Edge edge) { on_step(edge); });

    set_next_activation_time(Scheduling::unscheduled());
}

DiskControllerCard::~DiskControllerCard() = default;

void DiskControllerCard::step() {}

void DiskControllerCard::on_command(Edge edge)
{
    if (is_rising(edge))
    {
        // Bit 7:
        // Bit 6: STEP/
        // Bit 5: DIR/
        // Bit 0-4: Sector count

        const uint8_t data = *command;

        bool signal_for_step = (data & 0b01000000);
        bool dir_status = (data & 0b00100000);

        internal.step.set(signal_for_step, edge.time(), this);
        internal.dir = State(dir_status ? State::HIGH : State::LOW);

        auto old_sector = status.sector;
        status.sector = static_cast<uint32_t>(data & 0b11111);

        if (old_sector != status.sector)
        {
            status.index_in_sector = 0;
        }
        //

        update_card_status(edge.time());
    }
}

void DiskControllerCard::on_transfer_enabled(Edge edge)
{
    if (is_rising(edge))
    {
        const auto time = edge.time();
        const auto data_to_send = disk_data[status.index_in_sector];
        status.index_in_sector += 1;

        if (status.index_in_sector <= sizeof(disk_data))
        {
            output_data.set(data_to_send, time, this);
            schedule_available_data->launch(edge.time(), Scheduling::counter_type{100},
                                            change_schedule);
            status.sending_to_channel = true;
            status.reading = true;
            update_card_status(edge.time());
        }
        else
        {
            status.sending_to_channel = false;
            status.reading = false;
            update_card_status(edge.time());
        }
    }
}

void DiskControllerCard::on_end_of_transfer(Edge edge)
{
    if (is_rising(edge) && status.sending_to_channel)
    {
        status.sending_to_channel = false;
        status.reading = false;
        update_card_status(edge.time());
    }
}

std::vector<std::shared_ptr<Schedulable>> DiskControllerCard::get_sub_schedulables()
{
    return {schedule_status_changed, schedule_available_data};
}

void DiskControllerCard::on_activate(Edge edge)
{
    direction.set(State::HIGH, edge.time(), this);
    update_card_status(edge.time());
}

[[maybe_unused]] DiskControllerCard::Status DiskControllerCard::get_status() const
{
    return status;
}

void DiskControllerCard::on_step(Edge edge)
{
    if (is_falling(edge))
    {
        if (is_low(internal.dir))
        {
            if (status.track == 0)
            {
                status.track = 9;
            }
            else
            {
                status.track -= 1;
            }
        }
        else
        {
            status.track = (status.track + 1) % 10;
        }
    }
}

void DiskControllerCard::update_card_status(Scheduling::counter_type time)
{
    bool is_track_00 = status.track == 0;
    bool is_reading = status.reading;                 // Tentative name
    bool is_transferring = status.sending_to_channel; // Tentative name

    uint8_t current_status = ((is_track_00 ? 1 : 0) << 7) |
                             ((is_reading ? 1 : 0) << 1) |     // Tentative placement
                             ((is_transferring ? 1 : 0) << 0); // Tentative placement

    auto previous_status = card_status.get_value();
    card_status.set(current_status, time, this);

    if (previous_status != current_status)
    {
        schedule_status_changed->launch(time, Scheduling::counter_type{100}, change_schedule);
    }
}
