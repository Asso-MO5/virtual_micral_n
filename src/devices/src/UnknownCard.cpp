#include "UnknownCard.h"
#include <iostream>

#include "IOCard.h"
#include "Pluribus.h"

UnknownCard::UnknownCard(const Config& config)
    : scheduler{config.scheduler}, io_card{config.io_card}, configuration(config.configuration)
{
    io_card->ack_terminals[4].subscribe([this](Edge edge) { on_input_4(edge); });
    io_card->ack_terminals[5].subscribe([this](Edge edge) { on_input_5(edge); });
    io_card->ack_terminals[6].subscribe([this](Edge edge) { on_input_6(edge); });
    io_card->ack_terminals[7].subscribe([this](Edge edge) { on_input_7(edge); });

    set_next_activation_time(Scheduling::unscheduled());
}

UnknownCard::~UnknownCard() = default;

void UnknownCard::step() {}

using namespace std;

void UnknownCard::on_input_4(Edge edge)
{
    if (is_rising(edge))
    {
        cout << "Received on 0: " << hex
             << static_cast<uint32_t>(io_card->data_terminals[4].get_value()) << endl;
    }
}
void UnknownCard::on_input_5(Edge edge)
{
    if (is_rising(edge))
    {
        cout << "Received on 1: " << hex
             << static_cast<uint32_t>(io_card->data_terminals[5].get_value()) << endl;
    }
}
void UnknownCard::on_input_6(Edge edge)
{
    if (is_rising(edge))
    {
        const uint8_t data = io_card->data_terminals[6].get_value();

        if (data & 0b00000100)
        {
            cout << "Start the device" << endl;
        }

        assert((data - 0b00000100 == 0) &&
               "Signal not handled"); // TODO: temporary assert. This should be some sort of emulation error.
    }
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
        //  When the device is ready, it sends a 1 om bit 7 of channel 6
        // ROM outputs 1 on bit 6 and 5 of $f (3) (asks for transfer from the Device ?)
        // ROM outputs 1 on bit 0 and 4 of $c (0) (???)
        // ROM outputs 1 on bit 6 and 5 of $f (3) or'd with E (on 5 bits) (asks for transfer from the Device with an address ?)
        // ROM outputs $82 on port $d -> Most probably the counter for the Channel Card (input on $5).
        // ROM waits for un-asserted bit 0 and bit 1 on channel 6
        //  When the device is done with the transfer to the Channel Card, bit 0 and bit 1 of channel 6 are low
        // ROM reads on channel 5, it must by $94


        const uint8_t data = io_card->data_terminals[7].get_value();

        cout << "--> ";

        if (data & 0b01000000)
        {
            cout << "REQ - ";
        }
        if (data & 0b00100000)
        {
            cout << "HANSHK - ";
        }
        if (data & 0b00010000)
        {
            cout << "??? - ";
        }
        cout << "E: " << static_cast<uint32_t>(data & 0b1111) << endl;

        assert(((data & 0b10000000) == 0) &&
               "Signal not handled"); // TODO: temporary assert. This should be some sort of emulation error.
    }
}
