#ifndef MICRALN_PANELCONTROL_H
#define MICRALN_PANELCONTROL_H

#include <array>
#include <devices/src/CPU8008.h>

class Simulator;

class PanelControl
{
public:
    void display(Simulator& controller);

private:
    std::array<bool, 14> input_address{};
    std::array<bool, 8> input_data{};

    void display_address_line();
    void display_data_line();
    void display_control_line();
    void display_status_line(const CPU8008::OutputPins& output_pins);
    void display_av_init_line(Simulator& simulator);
};

#endif //MICRALN_PANELCONTROL_H
