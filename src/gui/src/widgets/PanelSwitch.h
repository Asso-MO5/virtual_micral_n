#ifndef MICRALN_PANELSWITCH_H
#define MICRALN_PANELSWITCH_H

namespace widgets
{
    enum ButtonType
    {
        TOGGLE,
        IMPULSE,
    };

    void display_control_button(const char* str_id, bool* value, ButtonType type);
}

#endif //MICRALN_PANELSWITCH_H
