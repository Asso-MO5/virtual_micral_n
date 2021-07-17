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
    connect(io_card.data_terminals[0], this).to(serial_card.input_data);
}
