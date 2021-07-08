#ifndef MICRALN_DISKCONTROLLERCARD_H
#define MICRALN_DISKCONTROLLERCARD_H

#include "DiskReader.h"

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/OwnedSignal.h>
#include <emulation_core/src/OwnedValue.h>
#include <emulation_core/src/Schedulable.h>
#include <memory>

class IOCommunicator;
class Pluribus;
class ScheduledSignal;

struct DiskControllerCardConfiguration
{
    uint8_t address_selection{};
    DiskReader::data_provider_type data_provider;
};

class DiskControllerCard : public SchedulableImpl
{
public:
    struct Config
    {
        Scheduling::change_schedule_cb change_schedule;
        std::shared_ptr<Pluribus> pluribus;
        DiskControllerCardConfiguration configuration;
    };

    struct Status
    {
        uint8_t track{};
        uint8_t sector{};
        bool reading{};
        bool sending_to_channel{};
    };

    explicit DiskControllerCard(const Config& config);
    ~DiskControllerCard() override;

    std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables() override;

    void step() override;

    [[nodiscard]] Status get_debug_data() const;

    // To the Stack/Channel card
    OwnedSignal available_data;
    OwnedValue<uint8_t> output_data;

    OwnedSignal change_pointer;
    OwnedValue<uint8_t> pointer_value_to_send;

    OwnedSignal change_counter;
    OwnedValue<uint8_t> counter_value;

    OwnedSignal direction;

    // From the Stack/Channel card
    OwnedSignal start_data_transfer;
    OwnedSignal stop_data_transfer;

    OwnedValue<uint8_t> received_pointer_value;

private:
    Scheduling::change_schedule_cb change_schedule;
    std::shared_ptr<Pluribus> pluribus;
    DiskControllerCardConfiguration configuration;

    std::shared_ptr<IOCommunicator> io_communicator;

    std::shared_ptr<ScheduledSignal> schedule_available_data;
    std::shared_ptr<ScheduledSignal> schedule_change_pointer;
    std::shared_ptr<ScheduledSignal> schedule_change_counter;

    Status status;

    struct Internal
    {
        OwnedValue<uint8_t> card_status;
        OwnedSignal step{};
        State dir{};
    } internal{};

    DiskReader disk_reader;

    void initialize_outward_signals();
    void initialize_inward_signals();
    void initialize_inner_signals();
    void initialize_io_communicator();

    [[nodiscard]] bool is_addressed(uint16_t address) const;

    void command_from_pluribus(uint16_t address, Scheduling::counter_type time);
    [[nodiscard]] uint8_t get_data_for_pluribus(uint16_t address) const;

    // Commands from the ROM
    void on_set_pointer(uint8_t data, Scheduling::counter_type time);
    void on_set_counter(uint8_t data, Scheduling::counter_type time);
    void on_activate(Scheduling::counter_type time);
    void on_command(uint8_t data, Scheduling::counter_type time);

    void on_transfer_enabled(Edge edge);
    void on_end_of_transfer(Edge edge);
    void on_step(Edge edge);

    void update_card_status(Scheduling::counter_type time);
};

#endif //MICRALN_DISKCONTROLLERCARD_H
