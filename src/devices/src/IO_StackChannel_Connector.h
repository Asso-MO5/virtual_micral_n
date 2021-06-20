#ifndef MICRALN_IO_STACKCHANNEL_CONNECTOR_H
#define MICRALN_IO_STACKCHANNEL_CONNECTOR_H

class IOCard;
class StackChannelCard;

class IO_StackChannel_Connector
{
public:
    IO_StackChannel_Connector(IOCard & io_card, StackChannelCard & stack_channel_card);
};

#endif //MICRALN_IO_STACKCHANNEL_CONNECTOR_H
