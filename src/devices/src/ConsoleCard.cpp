#include "ConsoleCard.h"

#include <devices/src/Pluribus.h>
#include <emulation_core/src/DataBus.h>

#include <utility>

ConsoleCard::ConsoleCard(std::shared_ptr<Pluribus> pluribus) : pluribus{std::move(pluribus)}
{
    set_next_activation_time(Scheduling::unscheduled());
    press_automatic();
}

ConsoleCard::Status ConsoleCard::get_status() const { return status; }

void ConsoleCard::step()
{
    status.data = pluribus->data_bus_d0_7->read();
    status.address = *pluribus->address_bus_s0_s13;
}

void ConsoleCard::press_automatic()
{
    status.automatic = true;
    status.stepping = false;
}
void ConsoleCard::press_stepping()
{
    status.automatic = false;
    status.stepping = true;
}

void ConsoleCard::press_trap() {}
