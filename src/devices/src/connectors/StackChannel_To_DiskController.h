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
        void from_disk_controller_to_stack_channel(DiskControllerCard& disk_controller,
                                                   StackChannelCard& stack_channel_card);
        void from_stack_channel_to_disk_controller(StackChannelCard& stack_channel_card,
                                                   DiskControllerCard& disk_controller);
    };
}

#endif //MICRALN_STACKCHANNEL_TO_DISKCONTROLLER_H
