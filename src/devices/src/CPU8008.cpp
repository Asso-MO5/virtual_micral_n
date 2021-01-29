#include "CPU8008.h"

CPU8008::CPU8008() {
    output_pins.state = State::STOPPED;
}

void CPU8008::step() {
    output_pins.state = State::T1I;
}

Scheduling::counter_type CPU8008::get_next_activation_time() const { return 0; }

const CPU8008::OutputPins& CPU8008::get_output_pins() const { return output_pins; }
