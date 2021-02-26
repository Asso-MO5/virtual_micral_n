#include "ConsoleCard.h"

#include <devices/src/Pluribus.h>

ConsoleCard::ConsoleCard(const std::shared_ptr<Pluribus>& pluribus) : pluribus{pluribus} {}

ConsoleCard::Status ConsoleCard::get_status() const { return status; }

void ConsoleCard::step() {}
