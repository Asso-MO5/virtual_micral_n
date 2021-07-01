#ifndef MICRALN_STACKCHANNEL_TO_DISKCONTROLLER_H
#define MICRALN_STACKCHANNEL_TO_DISKCONTROLLER_H

class StackChannelCard;
class DiskControllerCard;

namespace Connectors
{
    class StackChannel_To_DiskController
    {
    public:
        StackChannel_To_DiskController(StackChannelCard& stack_channel_card,
                                              DiskControllerCard& disk_controller);
    };
}

#endif //MICRALN_STACKCHANNEL_TO_DISKCONTROLLER_H
