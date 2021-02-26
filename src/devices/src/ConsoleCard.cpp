#include "ConsoleCard.h"

#include <devices/src/Pluribus.h>

#include <utility>

ConsoleCard::ConsoleCard(std::shared_ptr<Pluribus> pluribus) : pluribus{std::move(pluribus)}
{
    set_next_activation_time(Scheduling::unscheduled());
}

ConsoleCard::Status ConsoleCard::get_status() const { return status; }

void ConsoleCard::step() {}
