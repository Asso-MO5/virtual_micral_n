#ifndef MICRALN_GENERALADDRESSREGISTER_H
#define MICRALN_GENERALADDRESSREGISTER_H

#include <emulation_core/src/Edge.h>
#include <i8008/src/Constants8008.h>
#include <memory>

class CPU8008;
class Pluribus;

class GeneralAddressRegister
{
public:
    explicit GeneralAddressRegister(std::shared_ptr<CPU8008> cpu, std::shared_ptr<Pluribus>);

    [[nodiscard]] uint16_t get_latched_address() const;
    [[nodiscard]] Constants8008::CycleControl get_latched_cycle_control() const;

private:
    std::shared_ptr<CPU8008> cpu{};
    std::shared_ptr<Pluribus> pluribus{};
    uint16_t latched_address{};
    Constants8008::CycleControl latched_cycle_control{};

    void on_sync(Edge edge);
};

#endif //MICRALN_GENERALADDRESSREGISTER_H
