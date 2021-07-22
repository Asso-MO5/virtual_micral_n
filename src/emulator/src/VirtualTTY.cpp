#include "VirtualTTY.h"

#include <utility>

VirtualTTY::VirtualTTY() {}

void VirtualTTY::receive_char(char char_to_add)
{

    if (char_to_add == 0x11 || char_to_add == 0x12)
    {
        raw_output = true;
    }
    if (char_to_add == 0x13 || char_to_add == 0x14)
    {
        raw_output = false;
    }
    if (!raw_output)
    {
        char_to_add &= 0x7f;
    }

    full_content.push_back(char_to_add);
}

std::string_view VirtualTTY::content() const { return {full_content}; }

void VirtualTTY::emit_char(char char_to_emit)
{
    if (emitted_char_cb)
    {
        emitted_char_cb(char_to_emit);
    }
}

void VirtualTTY::set_emitted_char_cb(std::function<void(char)> new_emitted_char_cb)
{
    emitted_char_cb = std::move(new_emitted_char_cb);
}
