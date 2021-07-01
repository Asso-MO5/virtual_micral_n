#include "IO_To_DiskController.h"

#include "devices/src/DiskControllerCard.h"
#include "devices/src/IOCard.h"

namespace Connectors
{
    IO_To_DiskController::IO_To_DiskController(IOCard& io_card, DiskControllerCard& disk_controller)
    {
        from_io_to_controller(io_card, disk_controller);
        from_controller_to_io(io_card, disk_controller);
    }

    void IO_To_DiskController::from_controller_to_io(IOCard& io_card,
                                                     DiskControllerCard& disk_controller)
    {
        // Connected to I/O card IN $0/$FE
        const auto status_terminal = 2;
        io_card.data_terminals[status_terminal].request(this, Scheduling::counter_type{0});
        io_card.ack_terminals[status_terminal].request(this);

        disk_controller.card_status.subscribe(
                [&io_card, this](uint8_t, uint8_t new_value, Scheduling::counter_type time) {
                    io_card.data_terminals[status_terminal].set(new_value, time, this);
                });

        disk_controller.status_changed.subscribe([&io_card, this](Edge edge) {
            io_card.ack_terminals[status_terminal].apply(edge, this);
        });
    }

    void IO_To_DiskController::from_io_to_controller(IOCard& io_card,
                                                     DiskControllerCard& disk_controller)
    {
        // Connected to I/O card OUT $F
        const auto command_terminal = 7;
        disk_controller.receive_command.request(this);
        disk_controller.command.request(this, Scheduling::counter_type{0});

        io_card.data_terminals[command_terminal].subscribe(
                [&disk_controller, this](uint8_t, uint8_t new_value,
                                         Scheduling::counter_type time) {
                    disk_controller.command.set(new_value, time, this);
                });

        io_card.ack_terminals[command_terminal].subscribe([&disk_controller, this](Edge edge) {
            disk_controller.receive_command.apply(edge, this);
        });

        // Connected to I/O card OUT $E
        const auto control_terminal = 6;

        disk_controller.activate.request(this);
        io_card.ack_terminals[control_terminal].subscribe([&disk_controller, &io_card,
                                                           this](Edge edge) {
            const auto value = *io_card.data_terminals[control_terminal];
            const auto time = edge.time();
            const auto direction_bit = value & 0b00000100;
            disk_controller.activate.set(direction_bit ? State::HIGH : State::LOW, time, this);

            assert((value - 0b00000100 == 0) &&
                   "Signal not handled"); // TODO: temporary assert. This should be some sort of emulation error.
        });
    }
}