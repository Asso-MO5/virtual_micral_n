#include "SerialCard.h"

SerialCard::SerialCard(const Config& config)
    : change_schedule{config.change_schedule}, pluribus{config.pluribus},
      configuration{config.configuration}
{}

void SerialCard::step() {}

std::vector<std::shared_ptr<Schedulable>> SerialCard::get_sub_schedulables() { return {}; }
