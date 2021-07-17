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
    OwnedValue<uint8_t> input_data;

    // With the current implementation, the Serial Card concentrates all the flags
    // into a 8 bit word. It's a shortcut.
    //    OwnedSignal input_ready_PE;    // Bit 0
    //    OwnedSignal stop_bit_error_ES; // Bit 4
    //    OwnedSignal parity_error_EP;   // Bit 5
    //    OwnedSignal framing_error_SP;  // Bit 6
    //    OwnedSignal output_ready_PS;   // Bit 7
    OwnedValue<uint8_t> combined_status;

    // Non existent signal? At the moment, serves to signal the IO Card of the changed status
    OwnedSignal combined_status_changed;

    // From the IO Card
    OwnedSignal output_strobe_AS;
    OwnedValue<uint8_t> output_data;

private:
    Scheduling::change_schedule_cb change_schedule;
    std::shared_ptr<Pluribus> pluribus;
    SerialCardConfiguration configuration;

    void on_output(uint8_t value, Scheduling::counter_type time);
};

#endif //MICRALN_SERIALCARD_H
