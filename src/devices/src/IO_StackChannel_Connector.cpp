#include "IO_StackChannel_Connector.h"

#include "IOCard.h"
#include "StackChannelCard.h"

IO_StackChannel_Connector::IO_StackChannel_Connector(IOCard& io_card,
                                                     StackChannelCard& stack_channel_card)
{
    stack_channel_card.apply_counter.request(this);
    stack_channel_card.new_counter_value.request(this, Scheduling::schedulable_id{0});
    stack_channel_card.apply_pointer_address.request(this);
    stack_channel_card.new_pointer_address.request(this, Scheduling::schedulable_id{0});

    // From IO to StackChannel

    // Connect STPC/ or STR/ for Counter Change (PRx/ for value)
    const auto new_counter_terminal = 5;

    io_card.ack_terminals[new_counter_terminal].subscribe([&stack_channel_card, this](Edge edge) {
        stack_channel_card.apply_counter.apply(edge, this);
    });

    io_card.data_terminals[new_counter_terminal].subscribe(
            [&stack_channel_card, this](uint8_t, uint8_t new_value, Scheduling::counter_type time) {
                stack_channel_card.new_counter_value.set(new_value, time, this);
            });

    // Connect LOAD/ for Pointer Change (PAx/ Signal)
    const auto new_pointer_terminal = 4;

    io_card.ack_terminals[new_pointer_terminal].subscribe([&stack_channel_card, this](Edge edge) {
        stack_channel_card.apply_pointer_address.apply(edge, this);
    });

    io_card.data_terminals[new_pointer_terminal].subscribe(
            [&stack_channel_card, this](uint8_t, uint8_t new_value, Scheduling::counter_type time) {
                stack_channel_card.new_pointer_address.set(new_value, time, this);
            });

    // From StackChannel to IO

    // Connected to IN $0/$FF
    const auto io_0_ff = 3;
    io_card.data_terminals[io_0_ff].request(this, Scheduling::counter_type{0});
    io_card.ack_terminals[io_0_ff].request(this); // Not a real hardware signal?

    stack_channel_card.current_pointer_changed.subscribe(
            [&io_card, this](Edge edge) { io_card.ack_terminals[io_0_ff].apply(edge, this); });

    stack_channel_card.current_pointer_address.subscribe(
            [&io_card, this](uint8_t, uint8_t new_value, Scheduling::counter_type time) {
                io_card.data_terminals[io_0_ff].set(new_value, time, this);
            });
}
