#include "IO_To_StackChannel.h"

#include "devices/src/IOCard.h"
#include "devices/src/StackChannelCard.h"

namespace Connectors
{

    IO_To_StackChannel::IO_To_StackChannel(IOCard& io_card,
                                                         StackChannelCard& stack_channel_card)
    {
        from_io_to_stack_channel(io_card, stack_channel_card);
        from_stack_channel_to_io(io_card, stack_channel_card);

        // Invalid
        // The direction is supposed to be connected with the peripheral
        // TODO: connect the direction from the DiskController

        const auto control_terminal = 6;

        stack_channel_card.direction.request(this);
        io_card.ack_terminals[control_terminal].subscribe([&stack_channel_card, &io_card,
                                                           this](Edge edge) {
            const auto value = *io_card.data_terminals[control_terminal];
            const auto time = edge.time();
            const auto direction_bit = value & 0b00000100;
            stack_channel_card.direction.set(direction_bit ? State::HIGH : State::LOW, time, this);

            assert((value - 0b00000100 == 0) &&
                   "Signal not handled"); // TODO: temporary assert. This should be some sort of emulation error.
        });
    }

    void IO_To_StackChannel::from_stack_channel_to_io(IOCard& io_card,
                                                             StackChannelCard& stack_channel_card)
    {
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
    void IO_To_StackChannel::from_io_to_stack_channel(IOCard& io_card,
                                                             StackChannelCard& stack_channel_card)
    {
        // Connect STPC/ or STR/ for Counter Change (PRx/ for value)
        const auto new_counter_terminal = 5;

        stack_channel_card.apply_counter.request(this);
        stack_channel_card.new_counter_value.request(this, Scheduling::schedulable_id{0});

        io_card.ack_terminals[new_counter_terminal].subscribe(
                [&stack_channel_card, this](Edge edge) {
                    stack_channel_card.apply_counter.apply(edge, this);
                });

        io_card.data_terminals[new_counter_terminal].subscribe(
                [&stack_channel_card, this](uint8_t, uint8_t new_value,
                                            Scheduling::counter_type time) {
                    stack_channel_card.new_counter_value.set(new_value, time, this);
                });

        // Connect LOAD/ for Pointer Change (PAx/ Signal)
        const auto new_pointer_terminal = 4;

        stack_channel_card.apply_pointer_address.request(this);
        stack_channel_card.new_pointer_address.request(this, Scheduling::schedulable_id{0});

        io_card.ack_terminals[new_pointer_terminal].subscribe(
                [&stack_channel_card, this](Edge edge) {
                    stack_channel_card.apply_pointer_address.apply(edge, this);
                });

        io_card.data_terminals[new_pointer_terminal].subscribe(
                [&stack_channel_card, this](uint8_t, uint8_t new_value,
                                            Scheduling::counter_type time) {
                    stack_channel_card.new_pointer_address.set(new_value, time, this);
                });
    }
}