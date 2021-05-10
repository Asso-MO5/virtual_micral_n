#include "UnknownCard.h"

#include "IOCard.h"
#include "Pluribus.h"

UnknownCard::UnknownCard(const Config& config)
    : scheduler{config.scheduler}, io_card{config.io_card}, configuration(config.configuration)
{}

UnknownCard::~UnknownCard() = default;

void UnknownCard::step() {}
