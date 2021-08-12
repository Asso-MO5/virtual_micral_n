#ifndef MICRALN_IOCARD_H
#define MICRALN_IOCARD_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/OwnedSignal.h>
#include <emulation_core/src/OwnedValue.h>
#include <emulation_core/src/Schedulable.h>

#include <array>
#include <memory>
#include <vector>

class IOCommunicator;
class Pluribus;
class ScheduledSignal;

struct IOCardConfiguration
{
    enum Mode
    {
        Input_32_Output_32,
        Input_64,
        Output_64,
    };

    Mode mode{Input_32_Output_32};

    // 32/32 selects output on S11, S12, S13 and input on S2 (needs 4 bits)
    // - using S13|S12|S11|x|x|x|x|S2
    // 64 inputs selects on S3 to S7 for channel and S9 to S11 for group (needs 8 bits)
    // - using S11|S10|S9|S7|S6|S5|S4|S3
    // 64 outputs selects on S12 and S13 (needs 2 bits)
    // - using S13|S12|x|x|x|x|x|x
    uint8_t address_selection{};
};

namespace IOCardConstants
{
    const size_t TERMINAL_COUNT = 8;
}

class IOCard : public SchedulableImpl
{
public:
    struct Config
    {
        Scheduling::change_schedule_cb change_schedule;
        std::shared_ptr<Pluribus> pluribus;
        IOCardConfiguration configuration;
    };

    explicit IOCard(const Config& config);
    ~IOCard() override;

    std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables() override;

    void step() override;

    // For 32/32 cards, the 4 first are for inputs, the 4 others for outputs
    std::array<OwnedValue<uint8_t>, IOCardConstants::TERMINAL_COUNT> data_terminals;
    std::array<OwnedSignal, IOCardConstants::TERMINAL_COUNT> ack_terminals;
    // OwnedSignal interrupt_terminal; // TODO: Interrupt support for I/O card

private:
    Scheduling::change_schedule_cb change_schedule;
    std::shared_ptr<Pluribus> pluribus;
    IOCardConfiguration configuration;

    std::shared_ptr<IOCommunicator> io_communicator;

    size_t first_owned_terminal{};
    std::array<uint8_t, IOCardConstants::TERMINAL_COUNT> latched_input_data{};

    std::array<std::shared_ptr<ScheduledSignal>, IOCardConstants::TERMINAL_COUNT>
            scheduled_terminals_ACKs;

    void initialize_terminals();
    void initialize_io_communicator();

    void on_input_signal(uint8_t signal_index, Edge edge);

    [[nodiscard]] bool is_addressed(uint16_t address) const;
    [[nodiscard]] uint8_t address_to_output_number(uint16_t address) const;
    [[nodiscard]] uint8_t address_to_input_number(uint16_t address) const;
    void send_to_peripheral(uint16_t address, Scheduling::counter_type time);
    uint8_t get_from_peripheral(uint16_t address);
};

#endif //MICRALN_IOCARD_H
