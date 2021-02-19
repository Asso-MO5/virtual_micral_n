#ifndef MICRALN_PANELCONTROL_H
#define MICRALN_PANELCONTROL_H

#include <array>

class Simulator;

class PanelControl
{
public:
    void display(Simulator& controller);

private:
    bool interrupt_value = false;
    bool wait_value = false;

    std::array<bool, 8> inputs{};
};

#endif //MICRALN_PANELCONTROL_H
