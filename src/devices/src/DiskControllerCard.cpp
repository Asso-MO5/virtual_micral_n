#include "DiskControllerCard.h"

#include "IOCommunicator.h"
#include "Pluribus.h"

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
    : change_schedule{config.change_schedule}, pluribus{config.pluribus},
      configuration{config.configuration}
{
    // Outward signals
    direction.request(this);

    output_data.request(this, Scheduling::counter_type{0});
    schedule_available_data = std::make_shared<ScheduledSignal>(available_data);

    pointer_value_to_send.request(this, Scheduling::counter_type{0});
    schedule_change_pointer = std::make_shared<ScheduledSignal>(change_pointer);

    counter_value.request(this, Scheduling::counter_type{0});
    schedule_change_counter = std::make_shared<ScheduledSignal>(change_counter);

    // Inward signals
    //activate.subscribe([this](Edge edge) { on_activate(edge); });
    //receive_command.subscribe([this](Edge edge) { on_command(edge); });
    start_data_transfer.subscribe([this](Edge edge) { on_transfer_enabled(edge); });
    stop_data_transfer.subscribe([this](Edge edge) { on_end_of_transfer(edge); });

    // Inner signals
    internal.step.request(this);
    internal.step.subscribe([this](Edge edge) { on_step(edge); });
    internal.card_status.request(this, Scheduling::counter_type{0});

    initialize_io_communicator();

    set_next_activation_time(Scheduling::unscheduled());
}

void DiskControllerCard::initialize_io_communicator()
{
    IOCommunicatorConfiguration io_communicator_config{
            .on_need_data_for_pluribus =
                    [this](uint16_t address, Scheduling::counter_type) {
                        return get_data_for_pluribus(address);
                    },
            .on_acquire_from_pluribus =
                    [this](uint16_t address, Scheduling::counter_type time) {
                        command_from_pluribus(address, time);
                    },
            .addressed_predicate = [this](uint16_t address) { return is_addressed(address); }};
    io_communicator = make_shared<IOCommunicator>(
            IOCommunicator::Config{.change_schedule = change_schedule,
                                   .pluribus = pluribus,
                                   .configuration = io_communicator_config});
}

DiskControllerCard::~DiskControllerCard() = default;

void DiskControllerCard::step() {}

void DiskControllerCard::command_from_pluribus(uint16_t address, Scheduling::counter_type time)
{
    const auto high = (address & 0xff00) >> 8;
    const auto port = ((high & 0b00111110) >> 1) - 8;
    const auto data = (address & 0xff);

    switch (port)
    {
        case 0xC:
            on_set_pointer(data, time);
            break;
        case 0xD:
            on_set_counter(data, time);
            break;
        case 0xE:
            on_activate(time);
            break;
        case 0xF:
            on_command(data, time);
            break;
        default:
            assert(false && "Not a valid address");
    }
}

uint8_t DiskControllerCard::get_data_for_pluribus(uint16_t address) const
{
    const auto sub_address = (address & 0xff);

    switch (sub_address)
    {
        case 0xfe:
            return internal.card_status.get_value();
        case 0xff:
            return *received_pointer_value;
        default:
            assert(false && "Not a valid sub-address");
    }

    return 0;
}

void DiskControllerCard::on_set_pointer(uint8_t data, Scheduling::counter_type time)
{
    pointer_value_to_send.set(data, time, this);
    schedule_change_pointer->launch(time, 100, change_schedule);
}

void DiskControllerCard::on_set_counter(uint8_t data, Scheduling::counter_type time)
{
    counter_value.set(data, time, this);
    schedule_change_counter->launch(time, 100, change_schedule);
}

void DiskControllerCard::on_command(uint8_t data, Scheduling::counter_type time)
{
    bool signal_for_step = (data & 0b01000000); // Bit 6 is STEP/
    bool dir_status = (data & 0b00100000);      // Bit 5 is DIR/

    auto old_sector = status.sector;
    status.sector = static_cast<uint32_t>(data & 0b11111); // Bits 0-4 form the sector number.

    if (old_sector != status.sector)
    {
        status.index_in_sector = 0;
    }

    internal.step.set(signal_for_step, time, this);
    internal.dir = State(dir_status ? State::HIGH : State::LOW);

    update_card_status(time);
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
        }
        else
        {
            status.sending_to_channel = false;
            status.reading = false;
        }
        update_card_status(edge.time());
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

void DiskControllerCard::on_activate(Scheduling::counter_type time)
{
    direction.set(State::HIGH, time, this);
    update_card_status(time);
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

    internal.card_status.set(current_status, time, this);
}

bool DiskControllerCard::is_addressed(uint16_t address) const
{
    auto selection = configuration.address_selection;

    if (is_io_input_address(address))
    {
        uint8_t group = ((address >> 8) & 0b00001110) >> 1;
        if (group != 0)
        {
            return false;
        }

        uint8_t sub_address = (address & 0xff);

        return (sub_address == 0xfe) | (sub_address == 0xff);
    }
    else
    {
        uint8_t s13_to_s11 = ((address >> 11) << 5) & 0b11100000;
        return s13_to_s11 == (selection & 0b11100000);
    }
}

std::vector<std::shared_ptr<Schedulable>> DiskControllerCard::get_sub_schedulables()
{
    return {schedule_available_data, io_communicator, schedule_change_pointer,
            schedule_change_counter};
}
