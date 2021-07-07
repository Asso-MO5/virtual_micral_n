#include "StackChannel_To_DiskController.h"

#include "devices/src/DiskControllerCard.h"
#include "devices/src/StackChannelCard.h"

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
        disk_controller.start_data_transfer.request(this);
        disk_controller.stop_data_transfer.request(this);

        stack_channel_card.transfer_allowed.subscribe([&disk_controller, this](Edge edge) {
            disk_controller.start_data_transfer.apply(edge, this);
        });
        stack_channel_card.end_of_transfer.subscribe([&disk_controller, this](Edge edge) {
            disk_controller.stop_data_transfer.apply(edge, this);
        });

        // Connected to IN $0/$FF
        const auto io_0_ff = 3;
        disk_controller.received_pointer_value.request(this, Scheduling::counter_type{0});

        stack_channel_card.current_pointer_address.subscribe(
                [&disk_controller, this](uint8_t, uint8_t new_value,
                                         Scheduling::counter_type time) {
                    disk_controller.received_pointer_value.set(new_value, time, this);
                });
    }

    void StackChannel_To_DiskController::from_disk_controller_to_stack_channel(
            DiskControllerCard& disk_controller, StackChannelCard& stack_channel_card)
    {
        // Data transfer
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

        // Direction
        stack_channel_card.direction.request(this);
        disk_controller.direction.subscribe([&stack_channel_card, this](Edge edge) {
            stack_channel_card.direction.apply(edge, this);
        });

        // Connect LOAD/ for Pointer Change (PAx/ Signal)
        stack_channel_card.apply_pointer_address.request(this);
        stack_channel_card.new_pointer_address.request(this, Scheduling::schedulable_id{0});

        disk_controller.change_pointer.subscribe([&stack_channel_card, this](Edge edge) {
            stack_channel_card.apply_pointer_address.apply(edge, this);
        });

        disk_controller.pointer_value_to_send.subscribe(
                [&stack_channel_card, this](uint8_t, uint8_t new_value,
                                            Scheduling::counter_type time) {
                    stack_channel_card.new_pointer_address.set(new_value, time, this);
                });

        // Connect STPC/ or STR/ for Counter Change (PRx/ for value)
        stack_channel_card.apply_counter.request(this);
        stack_channel_card.new_counter_value.request(this, Scheduling::schedulable_id{0});

        disk_controller.change_counter.subscribe([&stack_channel_card, this](Edge edge) {
            stack_channel_card.apply_counter.apply(edge, this);
        });

        disk_controller.counter_value.subscribe(
                [&stack_channel_card, this](uint8_t, uint8_t new_value,
                                            Scheduling::counter_type time) {
                    stack_channel_card.new_counter_value.set(new_value, time, this);
                });
    }
}
