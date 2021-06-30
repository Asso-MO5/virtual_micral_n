#include "IO_DiskController_Connector.h"

#include "devices/src/DiskControllerCard.h"
#include "devices/src/IOCard.h"

IO_DiskController_Connector::IO_DiskController_Connector(IOCard& io_card, DiskControllerCard& unknown_card)
{
    // Connected to I/O card OUT $F
    const auto command_terminal = 7;
    unknown_card.receive_command.request(this);
    unknown_card.command.request(this, Scheduling::counter_type{0});

    io_card.data_terminals[command_terminal].subscribe(
            [&unknown_card, this](uint8_t, uint8_t new_value, Scheduling::counter_type time) {
                unknown_card.command.set(new_value, time, this);
            });

    io_card.ack_terminals[command_terminal].subscribe(
            [&unknown_card, this](Edge edge) { unknown_card.receive_command.apply(edge, this); });

    // Connected to I/O card IN $0/$FE
    const auto status_terminal = 2;
    io_card.data_terminals[status_terminal].request(this, Scheduling::counter_type{0});
    io_card.ack_terminals[status_terminal].request(this);

    unknown_card.card_status.subscribe(
            [&io_card, this](uint8_t, uint8_t new_value, Scheduling::counter_type time) {
                io_card.data_terminals[status_terminal].set(new_value, time, this);
            });

    unknown_card.status_changed.subscribe([&io_card, this](Edge edge) {
        io_card.ack_terminals[status_terminal].apply(edge, this);
    });
}
