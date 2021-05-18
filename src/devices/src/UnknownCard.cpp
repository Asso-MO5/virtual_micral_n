#include "UnknownCard.h"
#include <iostream>

#include "IOCard.h"
#include "Pluribus.h"
#include "StackChannelCard.h"

using namespace std;

namespace
{
    uint8_t disk_data[] = {
            'S', 'T', 'A', 'R', 'T', ' ', ' ', ' ', ' ', ' ', 'H', 'E', 'L', 'L', 'O', 'W', 'O',
            'R', 'L', 'D', 'H', 'E', 'L', 'L', 'O', 'W', 'O', 'R', 'L', 'D', 'H', 'E', 'L', 'L',
            'O', 'W', 'O', 'R', 'L', 'D', 'H', 'E', 'L', 'L', 'O', 'W', 'O', 'R', 'L', 'D', 'H',
            'E', 'L', 'L', 'O', 'W', 'O', 'R', 'L', 'D', 'H', 'E', 'L', 'L', 'O', 'W', 'O', 'R',
            'L', 'D', 'H', 'E', 'L', 'L', 'O', 'W', 'O', 'R', 'L', 'D', 'H', 'E', 'L', 'L', 'O',
            'W', 'O', 'R', 'L', 'D', 'H', 'E', 'L', 'L', 'O', 'W', 'O', 'R', 'L', 'D', 'H', 'E',
            'L', 'L', 'O', 'W', 'O', 'R', 'L', 'D', 'H', 'E', 'L', 'L', 'O', 'W', 'O', 'R', 'L',
            'D', 'H', 'E', 'L', 'L', 'O', 'W', 'O', 'R', 'L', 'D',
    };
}

UnknownCard::UnknownCard(const Config& config)
    : scheduler{config.scheduler}, io_card{config.io_card}, stack_channel{config.stack_channel},
      configuration(config.configuration)
{
    // Connected to IN $0/$FE
    io_card->data_terminals[2].request(this, Scheduling::counter_type{0});
    io_card->ack_terminals[2].request(this);

    // Connected to IN $0/$FF
    io_card->data_terminals[3].request(this, Scheduling::counter_type{0});
    io_card->ack_terminals[3].request(this);

    // Connected to OUT $F
    io_card->ack_terminals[7].subscribe([this](Edge edge) { on_input_7(edge); });

    stack_channel->input_data.request(this, Scheduling::counter_type{0});
    stack_channel->data_transfer.request(this);

    stack_channel->transfer_allowed.subscribe([this](Edge edge) { on_transfer_enabled(edge); });
    stack_channel->end_of_transfer.subscribe([this](Edge edge) { on_end_of_transfer(edge); });

    set_next_activation_time(Scheduling::unscheduled());
}

UnknownCard::~UnknownCard() = default;

void UnknownCard::step()
{
    const auto time = get_next_activation_time();

    if (next_signals_to_lower.time_for_ack_2 == time)
    {
        io_card->ack_terminals[2].set(State::LOW, time, this);
        next_signals_to_lower.time_for_ack_2 = Scheduling::unscheduled();
    }

    if (next_signals_to_lower.time_for_ack_3 == time)
    {
        io_card->ack_terminals[3].set(State::LOW, time, this);
        next_signals_to_lower.time_for_ack_3 = Scheduling::unscheduled();
    }

    if (next_signals_to_lower.time_for_data_transfer == time)
    {
        stack_channel->data_transfer.set(State::LOW, time, this);
        next_signals_to_lower.time_for_data_transfer = Scheduling::unscheduled();
    }

    auto next_activation = std::min(next_signals_to_lower.time_for_data_transfer,
                                    next_signals_to_lower.time_for_ack_2);
    next_activation = std::min(next_activation, next_signals_to_lower.time_for_ack_3);
    set_next_activation_time(next_activation);
}

void UnknownCard::on_input_7(Edge edge)
{
    if (is_rising(edge))
    {
        // Bit 7:
        // Bit 6: I/O Ready (can be a short pulse) -> Wake up the unknown card
        // Bit 5: Next Chunk
        // Bit 4: First round of E
        // Bit 0-3: Counter

        // ROM outputs 1 on bit 6 of $f (3) (in pulses)
        // ROM waits for asserted bit 7 on channel 6
        //  When the device is ready, it sends a 1 om bit 7 of channel 6 (=2)
        // ROM outputs 1 on bit 6 and 5 of $f (3 -> 7) (asks for transfer from the Device ?)
        // ROM outputs 1 on bit 0 and 4 of $c (0) (???)
        // ROM outputs 1 on bit 6 and 5 of $f (3 -> 7) or'd with E (on 5 bits) (asks for transfer from the Device with an address ?)
        // ROM outputs $82 on port $d -> Most probably the counter for the Channel Card (input on $5).
        // ROM waits for un-asserted bit 0 and bit 1 on channel 6
        //  When the device is done with the transfer to the Channel Card, bit 0 and bit 1 of channel 6 are low
        // ROM reads on channel 5, it must by $94

        const uint8_t data = io_card->data_terminals[7].get_value();

        if (data & 0b01000000)
        {
            cout << "--> ";
            cout << "REQ - ";

            if (!status.is_ready)
            {
                status.is_ready = true;
                io_card->data_terminals[2].set(0b10000011, edge.time(), this);
                io_card->ack_terminals[2].set(State::HIGH, edge.time(), this);

                next_signals_to_lower.time_for_ack_2 = edge.time() + Scheduling::counter_type{100};
                set_next_activation_time(edge.time() + Scheduling::counter_type{100});

                scheduler.change_schedule(get_id());
            }
            if (data & 0b00100000)
            {
                cout << "HANDSHAKE - ";
            }
            if (data & 0b00010000)
            {
                if (status.is_ready && !status.sending_to_channel)
                {
                    cout << "GO - ";
                    status.sending_to_channel = true;
                    status.index_on_disk = 0;

                    // Sends $00 on the INP $/$ff channel.
                    // No idea what it describes, this is just to avoid a $94 constant status
                    io_card->data_terminals[3].set(0b00000000, edge.time(), this);
                    io_card->ack_terminals[3].set(State::HIGH, edge.time(), this);

                    next_signals_to_lower.time_for_ack_3 =
                            edge.time() + Scheduling::counter_type{100};

                    io_card->data_terminals[2].set(0b00000011, edge.time(), this);
                    io_card->ack_terminals[2].set(State::HIGH, edge.time(), this);

                    next_signals_to_lower.time_for_ack_2 =
                            edge.time() + Scheduling::counter_type{100};
                    set_next_activation_time(edge.time() + Scheduling::counter_type{100});

                    scheduler.change_schedule(get_id());
                }
                else
                {
                    cout << "ALREADY GOING - ";
                }
            }
            cout << "E: " << static_cast<uint32_t>(data & 0b1111);
            cout << endl;
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

            stack_channel->input_data.set(data_to_send, time, this);
            stack_channel->data_transfer.set(State::HIGH, time, this);

            next_signals_to_lower.time_for_data_transfer =
                    edge.time() + Scheduling::counter_type{100};
            set_next_activation_time(edge.time() + Scheduling::counter_type{100});

            scheduler.change_schedule(get_id());

            if (status.index_on_disk >= sizeof(disk_data))
            {
                status.sending_to_channel = false;
                status.is_ready = false;
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

        // Sends $94 on the INP $/$ff channel.
        // No idea what it describes, but that's what the ROM expects
        io_card->data_terminals[3].set(0b10010100, edge.time(), this);
        io_card->ack_terminals[3].set(State::HIGH, edge.time(), this);

        next_signals_to_lower.time_for_ack_3 = edge.time() + Scheduling::counter_type{100};
        //set_next_activation_time(edge.time() + Scheduling::counter_type{100});
        //scheduler.change_schedule(get_id());

        io_card->data_terminals[2].set(0b00000000, edge.time(), this);
        io_card->ack_terminals[2].set(State::HIGH, edge.time(), this);

        next_signals_to_lower.time_for_ack_2 = edge.time() + Scheduling::counter_type{100};
        set_next_activation_time(edge.time() + Scheduling::counter_type{100});
        scheduler.change_schedule(get_id());
    }
}
