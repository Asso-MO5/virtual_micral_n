#ifndef MICRALN_PANELLED_H
#define MICRALN_PANELLED_H

namespace widgets
{
    enum LedColor
    {
        GREEN,
        RED,
    };

    void display_led(float intensity, LedColor color_type);
}

#endif //MICRALN_PANELLED_H
