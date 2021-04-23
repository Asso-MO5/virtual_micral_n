#ifndef MICRALN_PANELCONTROL_H
#define MICRALN_PANELCONTROL_H

#include <array>

class Simulator;
class ConsoleCard;

class PanelControl
{
public:
    void display(Simulator& controller);

    enum DisplayMode
    {
        Instant,
        Remanence,
    };
    void set_display_mode(DisplayMode mew_display_mode);

private:
    std::array<bool, 14> input_address{};
    std::array<bool, 8> input_data{};
    std::array<bool, 6> control_switches{};
    DisplayMode display_mode{Remanence};

    void display_address_line(ConsoleCard& console_card);
    void display_data_line(ConsoleCard& console_card);
    void display_control_line(ConsoleCard& console_card);
    static void display_status_line(ConsoleCard& console_card);
    static void display_av_init_line(ConsoleCard& console_card);
};

#endif //MICRALN_PANELCONTROL_H
