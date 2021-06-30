#ifndef MICRALN_IO_TO_STACKCHANNEL_H
#define MICRALN_IO_TO_STACKCHANNEL_H

class IOCard;
class StackChannelCard;

namespace Connectors
{
    class IO_To_StackChannel
    {
    public:
        IO_To_StackChannel(IOCard& io_card, StackChannelCard& stack_channel_card);

    private:
        void from_io_to_stack_channel(IOCard& io_card, StackChannelCard& stack_channel_card);
        void from_stack_channel_to_io(IOCard& io_card, StackChannelCard& stack_channel_card);
    };
}

#endif //MICRALN_IO_TO_STACKCHANNEL_H
