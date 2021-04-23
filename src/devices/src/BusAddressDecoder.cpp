#include "BusAddressDecoder.h"

#include "CPU8008.h"
#include "Pluribus.h"

BusAddressDecoder::BusAddressDecoder(std::shared_ptr<CPU8008> cpu, std::shared_ptr<Pluribus> pluribus)
    : cpu{std::move(cpu)}, pluribus{std::move(pluribus)}
{
    this->cpu->register_sync_trigger([this](Edge edge) { on_sync(edge); });

    this->pluribus->cc0.request(this);
    this->pluribus->cc1.request(this);
    this->pluribus->address_bus_s0_s13.request(this, 0);
}

void BusAddressDecoder::on_sync(Edge edge)
{
    if (edge == Edge{Edge::Front::FALLING})
    {
        auto cpu_state = *cpu->output_pins.state;

        // TODO: Verify if the address is also sent on the BUS after an interruption
        if (cpu_state == Constants8008::CpuState::T1 || cpu_state == Constants8008::CpuState::T1I)
        {
            latched_address &= 0xff00;
            latched_address |= cpu->data_pins.get_value();
        }
        else if (cpu_state == Constants8008::CpuState::T2)
        {
            auto read_value = cpu->data_pins.get_value();

            latched_address &= 0x00ff;
            latched_address |= (read_value & 0x3f) << 8;

            latched_cycle_control =
                    static_cast<Constants8008::CycleControl>(read_value & 0b11000000);

            auto cc0 = (read_value & 0b10000000) >> 7;
            auto cc1 = (read_value & 0b01000000) >> 6;

            auto time = edge.time();

            pluribus->address_bus_s0_s13.set(latched_address, time, this);
            pluribus->cc0.set(cc0, time, this);
            pluribus->cc1.set(cc1, time, this);
        }
    }
}

uint16_t BusAddressDecoder::get_latched_address() const { return latched_address; }

Constants8008::CycleControl BusAddressDecoder::get_latched_cycle_control() const
{
    return latched_cycle_control;
}
