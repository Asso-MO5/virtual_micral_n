#ifndef MICRALN_STACKCHANNEL_DISKCONTROLLER_CONNECTOR_H
#define MICRALN_STACKCHANNEL_DISKCONTROLLER_CONNECTOR_H

class StackChannelCard;
class DiskControllerCard;

namespace Connectors
{
    class StackChannel_DiskController_Connector
    {
    public:
        StackChannel_DiskController_Connector(StackChannelCard& stack_channel_card,
                                              DiskControllerCard& unknown_card);
    };
}

#endif //MICRALN_STACKCHANNEL_DISKCONTROLLER_CONNECTOR_H
