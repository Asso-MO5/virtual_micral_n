#ifndef MICRALN_VIRTUALTTY_H
#define MICRALN_VIRTUALTTY_H

#include <functional>
#include <string>
#include <string_view>

class VirtualTTY
{
public:
    VirtualTTY();

    [[nodiscard]] std::string_view content() const;

    void receive_char(char char_to_add);
    void emit_char(char char_to_emit);

    // Not in the constructor because it needs to be created before the SerialCard to pass
    // the receive_char() function to the SerialCard.
    // It could have been the other way around, changing then the SerialCard. But SerialCard
    // being a Card, adhere to their own configuration style.
    void set_emitted_char_cb(std::function<void(char)> new_emitted_char_cb);

private:
    std::string full_content;
    std::function<void(char)> emitted_char_cb;
};

#endif //MICRALN_VIRTUALTTY_H
