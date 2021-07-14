#include "StackChannel_To_DiskController.h"

#include "devices/src/DiskControllerCard.h"
#include "devices/src/StackChannelCard.h"

#include <emulation_core/src/SignalConnect.h>
#include <emulation_core/src/ValueConnect.h>

namespace Connectors
{
    StackChannel_To_DiskController::StackChannel_To_DiskController(
            StackChannelCard& stack_channel_card, DiskControllerCard& disk_controller)
    {
        from_disk_controller_to_stack_channel(disk_controller, stack_channel_card);
        from_stack_channel_to_disk_controller(stack_channel_card, disk_controller);
    }

    void StackChannel_To_DiskController::from_stack_channel_to_disk_controller(
            StackChannelCard& stack_channel_card, DiskControllerCard& disk_controller)
    {
        // Transfer control
        connect(stack_channel_card.transfer_allowed, this).to(disk_controller.start_data_transfer);
        connect(stack_channel_card.end_of_transfer, this).to(disk_controller.stop_data_transfer);

        // Connected to IN $0/$FF
        connect(stack_channel_card.current_pointer_address, this)
                .mask<uint8_t>(0xff, 0)
                .to(disk_controller.received_pointer_value);
    }

    void StackChannel_To_DiskController::from_disk_controller_to_stack_channel(
            DiskControllerCard& disk_controller, StackChannelCard& stack_channel_card)
    {
        // Data transfer
        connect(disk_controller.available_data, this).to(stack_channel_card.data_transfer);
        connect(disk_controller.output_data, this).to(stack_channel_card.input_data);

        // Direction
        connect(disk_controller.direction, this).to(stack_channel_card.direction);

        // Connect LOAD/ for Pointer Change (PAx/ Signal)
        connect(disk_controller.change_pointer, this).to(stack_channel_card.apply_pointer_address);
        connect(disk_controller.pointer_value_to_send, this)
                .mask<uint16_t>(0xff, 0)
                .to(stack_channel_card.new_pointer_address);

        // Connect STPC/ or STR/ for Counter Change (PRx/ for value)
        connect(disk_controller.change_counter, this).to(stack_channel_card.apply_counter);
        connect(disk_controller.counter_value, this)
                .mask<uint16_t>(0xff, 0)
                .to(stack_channel_card.new_counter_value);
    }
}
