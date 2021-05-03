#ifndef MICRALN_DATAONMDBUSHOLDER_H
#define MICRALN_DATAONMDBUSHOLDER_H

#include <emulation_core/src/Scheduling.h>

class Pluribus;

class DataOnMDBusHolder
{
public:
    explicit DataOnMDBusHolder(Pluribus& pluribus);
    void take_bus(Scheduling::counter_type time, uint8_t data);
    void place_data(Scheduling::counter_type time);
    void release_bus(Scheduling::counter_type time);

    [[nodiscard]] bool is_holding_bus() const;

private:
    Pluribus& pluribus;
    uint8_t latched_data{};
    bool is_emitting_data{};
};

#endif //MICRALN_DATAONMDBUSHOLDER_H
