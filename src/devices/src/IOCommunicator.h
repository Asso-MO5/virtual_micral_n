#ifndef MICRALN_IOCOMMUNICATOR_H
#define MICRALN_IOCOMMUNICATOR_H

#include <cstdint>
#include <functional>
#include <memory>

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>
#include <emulation_core/src/Scheduling.h>

class DataOnMDBusHolder;
class Pluribus;
class ScheduledAction;

struct IOCommunicatorConfiguration
{
    std::function<uint8_t(uint16_t)> on_need_data_for_pluribus;
    std::function<void(uint16_t, Scheduling::counter_type)> on_acquire_from_pluribus;
    std::function<bool(uint16_t)> addressed_predicate;
};

class IOCommunicator : public SchedulableImpl
{
public:
    struct Config
    {
        Scheduling::change_schedule_cb change_schedule;
        std::shared_ptr<Pluribus> pluribus;
        IOCommunicatorConfiguration configuration;
    };

    explicit IOCommunicator(const Config& config);
    ~IOCommunicator() override;

    void step() override;
    std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables() override;

private:
    // From parameters
    std::shared_ptr<Pluribus> pluribus;
    Scheduling::change_schedule_cb change_schedule;

    IOCommunicatorConfiguration configuration;

    // Internal
    std::unique_ptr<DataOnMDBusHolder> output_data_holder;
    std::shared_ptr<ScheduledAction> place_data_on_pluribus;

    void on_t2(Edge edge);
    void on_t3(Edge edge);
};

constexpr inline bool is_io_input_address(uint16_t address)
{
    return (address & 0b11000000000000) == 0;
}

#endif //MICRALN_IOCOMMUNICATOR_H
