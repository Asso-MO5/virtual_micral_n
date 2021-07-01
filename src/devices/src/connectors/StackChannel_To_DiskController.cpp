#include "StackChannel_To_DiskController.h"

#include "devices/src/DiskControllerCard.h"
#include "devices/src/StackChannelCard.h"

namespace Connectors
{
    StackChannel_To_DiskController::StackChannel_To_DiskController(
            StackChannelCard& stack_channel_card, DiskControllerCard& disk_controller)
    {
        // From DiskController to StackChannel
        stack_channel_card.data_transfer.request(this);
        stack_channel_card.input_data.request(this, Scheduling::counter_type{0});

        disk_controller.output_data.subscribe(
                [&stack_channel_card, this](uint8_t, uint8_t new_value,
                                            Scheduling::counter_type time) {
                    stack_channel_card.input_data.set(new_value, time, this);
                });

        disk_controller.available_data.subscribe([&stack_channel_card, this](Edge edge) {
            stack_channel_card.data_transfer.apply(edge, this);
        });

        stack_channel_card.direction.request(this);
        disk_controller.direction.subscribe([&stack_channel_card, this](Edge edge) {
            stack_channel_card.direction.apply(edge, this);
        });

        // From the StackChannel to the DiskController
        disk_controller.start_data_transfer.request(this);
        disk_controller.stop_data_transfer.request(this);

        stack_channel_card.transfer_allowed.subscribe([&disk_controller, this](Edge edge) {
            disk_controller.start_data_transfer.apply(edge, this);
        });
        stack_channel_card.end_of_transfer.subscribe([&disk_controller, this](Edge edge) {
            disk_controller.stop_data_transfer.apply(edge, this);
        });
    }
}
