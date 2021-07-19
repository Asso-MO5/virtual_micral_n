#include "VirtualTTY.h"

#include <gmock/gmock.h>

using namespace testing;

TEST(VirtualTTY, can_recieve_a_char)
{
    char probe_char = 0;
    VirtualTTY tty{};
    tty.set_emitted_char_cb([&probe_char](char emitted_char) { probe_char = emitted_char; });

    tty.receive_char('a');

    ASSERT_THAT(tty.content(), Eq("a"));
    ASSERT_THAT(probe_char, Eq(0));
}

TEST(VirtualTTY, can_send_a_char_with_no_echo)
{
    char probe_char = 0;
    VirtualTTY tty{};
    tty.set_emitted_char_cb([&probe_char](char emitted_char) { probe_char = emitted_char; });

    tty.emit_char('b');

    ASSERT_THAT(tty.content(), Eq(""));
    ASSERT_THAT(probe_char, Eq('b'));
}
