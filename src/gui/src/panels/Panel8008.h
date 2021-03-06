#ifndef MICRALN_PANEL8008_H
#define MICRALN_PANEL8008_H

#include <cstdint>

class Simulator;

void display_8008_panel(const Simulator& simulator, uint64_t average_frequency);

#endif //MICRALN_PANEL8008_H
