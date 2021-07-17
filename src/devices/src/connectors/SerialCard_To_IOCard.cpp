#include "SerialCard_To_IOCard.h"

#include "devices/src/IOCard.h"
#include "devices/src/SerialCard.h"
#include <emulation_core/src/SignalConnect.h>
#include <emulation_core/src/ValueConnect.h>

Connectors::SerialCard_To_IOCard::SerialCard_To_IOCard(SerialCard& serial_card, IOCard& io_card)
{
    // From IO to Serial
    connect(io_card.data_terminals[4], this).to(serial_card.output_data);

    // From Serial to IO
    connect(serial_card.input_data, this).to(io_card.data_terminals[0]);
    connect(serial_card.input_strobe_VE, this).to(io_card.ack_terminals[0]);

    connect(serial_card.combined_status, this).to(io_card.data_terminals[1]);
    connect(serial_card.combined_status_changed, this).to(io_card.ack_terminals[1]);
}
