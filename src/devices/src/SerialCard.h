#ifndef MICRALN_SERIALCARD_H
#define MICRALN_SERIALCARD_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/OwnedSignal.h>
#include <emulation_core/src/OwnedValue.h>
#include <emulation_core/src/Schedulable.h>
#include <memory>

class Pluribus;

struct SerialCardConfiguration
{
};

class SerialCard : public SchedulableImpl
{
public:
    struct Config
    {
        Scheduling::change_schedule_cb change_schedule;
        std::shared_ptr<Pluribus> pluribus;
        SerialCardConfiguration configuration;
    };

    explicit SerialCard(const Config& config);

    void step() override;
    std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables() override;

    // To the IO Card
    OwnedSignal input_strobe_VE;
    OwnedSignal input_ready_PE;
    OwnedSignal framing_error_SP;
    OwnedSignal stop_bit_error_ES;
    OwnedSignal parity_error_EP;
    OwnedValue<uint8_t> input_data;

    // From the IO Card
    OwnedSignal output_strobe_AS;
    OwnedSignal output_ready_PS;
    OwnedValue<uint8_t> output_data;

private:
    Scheduling::change_schedule_cb change_schedule;
    std::shared_ptr<Pluribus> pluribus;
    SerialCardConfiguration configuration;
};

#endif //MICRALN_SERIALCARD_H
