#ifndef MICRALN_PANELCONTROL_H
#define MICRALN_PANELCONTROL_H

#include <array>
#include <cstdint>

class Simulator;
class ConsoleCard;

class PanelControl
{
public:
    void display(Simulator& simulator);

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

    // Hacky way to start the monitor by automation of the manipulation
    // Press Shift-] (on a QWERTY keyboard) to advance through the 8 steps.
    // TODO: This will need to be transformed into actual possibility of scripting the simulator
    void hack_for_monitor(ConsoleCard& console_card);
    void set_hack_data(ConsoleCard& console_card, std::uint8_t data);
};

#endif //MICRALN_PANELCONTROL_H
