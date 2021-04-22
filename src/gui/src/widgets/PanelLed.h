#ifndef MICRALN_PANELLED_H
#define MICRALN_PANELLED_H

namespace widgets
{
    enum LedColor
    {
        GREEN,
    };

    void display_led(bool value, LedColor color_type, float intensity = 1.f);
}

#endif //MICRALN_PANELLED_H
