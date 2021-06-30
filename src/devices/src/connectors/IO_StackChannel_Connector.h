#ifndef MICRALN_IO_STACKCHANNEL_CONNECTOR_H
#define MICRALN_IO_STACKCHANNEL_CONNECTOR_H

class IOCard;
class StackChannelCard;

namespace Connectors
{
    class IO_StackChannel_Connector
    {
    public:
        IO_StackChannel_Connector(IOCard& io_card, StackChannelCard& stack_channel_card);

    private:
        void from_io_to_stack_channel(IOCard& io_card, StackChannelCard& stack_channel_card);
        void from_stack_channel_to_io(IOCard& io_card, StackChannelCard& stack_channel_card);
    };
}

#endif //MICRALN_IO_STACKCHANNEL_CONNECTOR_H
