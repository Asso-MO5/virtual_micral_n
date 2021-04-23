#ifndef MICRALN_BUSADDRESSDECODER_H
#define MICRALN_BUSADDRESSDECODER_H

#include "Constants8008.h"

#include <emulation_core/src/Edge.h>
#include <memory>

class CPU8008;
class Pluribus;

class BusAddressDecoder
{
public:
    explicit BusAddressDecoder(std::shared_ptr<CPU8008> cpu, std::shared_ptr<Pluribus>);

    [[nodiscard]] uint16_t get_latched_address() const;
    [[nodiscard]] Constants8008::CycleControl get_latched_cycle_control() const;

private:
    std::shared_ptr<CPU8008> cpu{};
    std::shared_ptr<Pluribus> pluribus{};
    uint16_t latched_address{};
    Constants8008::CycleControl latched_cycle_control{};

    void on_sync(Edge edge);
};

#endif //MICRALN_BUSADDRESSDECODER_H
