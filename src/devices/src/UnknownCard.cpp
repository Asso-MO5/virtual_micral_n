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
        // Bit 6: I/O Ready (can be a short pulse)
        // Bit 5: Next Chunk
        // Bit 4: First round of E
        // Bit 0-3: Counter

        const uint8_t data = io_card->data_terminals[7].get_value();

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
        cout << "Counter: " << static_cast<uint32_t>(data & 0b1111) << endl;

        assert(((data & 0b10000000) == 0) &&
               "Signal not handled"); // TODO: temporary assert. This should be some sort of emulation error.
    }
}
