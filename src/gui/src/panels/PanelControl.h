#ifndef MICRALN_PANELCONTROL_H
#define MICRALN_PANELCONTROL_H

#include <array>

class Simulator;
class ConsoleCard;

class PanelControl
{
public:
    void display(Simulator& controller);

private:
    std::array<bool, 14> input_address{};
    std::array<bool, 8> input_data{};
    std::array<bool, 6> control_switches{};

    void display_address_line(ConsoleCard& console_card);
    void display_data_line(ConsoleCard& console_card);
    void display_control_line(ConsoleCard& console_card);
    static void display_status_line(ConsoleCard& console_card);
    static void display_av_init_line(Simulator& simulator);
};

#endif //MICRALN_PANELCONTROL_H
