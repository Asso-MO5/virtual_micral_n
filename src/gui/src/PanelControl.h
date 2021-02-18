#ifndef MICRALN_PANELCONTROL_H
#define MICRALN_PANELCONTROL_H

#include <array>

class PanelControl
{
public:
    void display();

private:
    bool interrupt_value = false;
    bool wait_value = false;

    std::array<bool, 8> inputs{};
};

#endif //MICRALN_PANELCONTROL_H
