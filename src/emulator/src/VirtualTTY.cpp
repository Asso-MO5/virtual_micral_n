#include "VirtualTTY.h"

#include <utility>

VirtualTTY::VirtualTTY() {}

void VirtualTTY::receive_char(char char_to_add) { full_content.push_back(char_to_add); }

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
