#ifndef MICRALN_DISKCONTROLLERCARD_H
#define MICRALN_DISKCONTROLLERCARD_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/OwnedSignal.h>
#include <emulation_core/src/OwnedValue.h>
#include <emulation_core/src/Schedulable.h>
#include <memory>

class IOCard;
class ScheduledSignal;

struct DiskControllerCardConfiguration
{
};

class DiskControllerCard : public SchedulableImpl
{
public:
    struct Config
    {
        Scheduling::change_schedule_cb change_schedule;
        DiskControllerCardConfiguration configuration;
    };

    struct Status
    {
        uint8_t track{};
        uint8_t sector{};
        bool ready{};
        bool sending_to_channel{};
        bool received_2{};
        bool received_4{};
        bool received_6_once{};
        bool received_6_twice{};
        uint8_t index_on_disk{};
    };

    explicit DiskControllerCard(const Config& config);
    ~DiskControllerCard() override;

    std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables() override;

    void step() override;

    Status get_status() const;

    // From I/O card
    OwnedValue<uint8_t> command;
    OwnedSignal receive_command;
    OwnedSignal activate;

    // To I/O card
    OwnedValue<uint8_t> card_status;
    OwnedSignal status_changed;

    // To the Stack/Channel card
    OwnedSignal available_data;
    OwnedValue<uint8_t> output_data;

    OwnedSignal direction;

    // From the Stack/Channel card
    OwnedSignal start_data_transfer;
    OwnedSignal stop_data_transfer;

private:
    Scheduling::change_schedule_cb change_schedule;
    DiskControllerCardConfiguration configuration;

    std::shared_ptr<ScheduledSignal> schedule_status_changed;
    std::shared_ptr<ScheduledSignal> schedule_available_data;

    Status status;

    void on_command(Edge edge);

    void on_transfer_enabled(Edge edge);
    void on_end_of_transfer(Edge edge);
    void on_activate(Edge edge);
};

#endif //MICRALN_DISKCONTROLLERCARD_H
