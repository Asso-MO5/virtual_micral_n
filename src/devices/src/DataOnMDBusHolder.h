#ifndef MICRALN_DATAONMDBUSHOLDER_H
#define MICRALN_DATAONMDBUSHOLDER_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>
#include <emulation_core/src/Scheduling.h>

class Pluribus;
class ScheduledAction;

class DataOnMDBusHolder : public SchedulableImpl
{
public:
    explicit DataOnMDBusHolder(const std::shared_ptr<Pluribus>& pluribus,
                               Scheduling::change_schedule_cb change_schedule,
                               Scheduling::counter_type delay);

    void step() override;
    std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables() override;

    void place(Scheduling::counter_type time, uint8_t data);

private:
    std::shared_ptr<Pluribus> pluribus;
    Scheduling::change_schedule_cb change_schedule;
    Scheduling::counter_type delay;

    std::shared_ptr<ScheduledAction> place_data_on_pluribus;

    uint8_t latched_data{};
    bool has_data_to_send{};
    bool owns_bus{};

    void on_t3(Edge edge);
    void place_data(Scheduling::counter_type time);
    void release_bus(Scheduling::counter_type time);
    void on_sub(Edge edge);
    void take_bus(Scheduling::counter_type time);
};

#endif //MICRALN_DATAONMDBUSHOLDER_H
