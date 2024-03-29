#ifndef MICRALN_STACKCHANNELCARD_H
#define MICRALN_STACKCHANNELCARD_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/OwnedSignal.h>
#include <emulation_core/src/OwnedValue.h>
#include <emulation_core/src/Schedulable.h>

#include <memory>
#include <vector>

class IOCommunicator;
class Pluribus;
class ScheduledSignal;

struct StackChannelCardConfiguration
{
    enum Mode
    {
        Stack,
        Channel,
    };

    Mode mode{Stack};
    uint16_t memory_size{};
    uint8_t input_address{};
    uint8_t output_address{};
};

class StackChannelCard : public SchedulableImpl
{
public:
    struct Config
    {
        Scheduling::change_schedule_cb change_schedule;
        std::shared_ptr<Pluribus> pluribus;
        StackChannelCardConfiguration configuration;
    };

    explicit StackChannelCard(const Config& config);
    ~StackChannelCard() override;

    std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables() override;

    void step() override;

    // Unknown direction with I/O
    // It could be an input and that would explain with the ROM sends 0x11 (1 for direction,
    // and one for "transfer cycle" ?
    OwnedSignal transfer_cycle; // TC/

    // Output with... I/O or Peripheral, depending on the paragraph
    // At now, the bet is on Peripheral because it allows it to understand the asked transfer
    // is finished (rather than stalling on the lack of ACK/), and ROM is polling for a value
    // that seems to also be used to ask for the readiness of the peripheral
    // Note that the I/O card terminal could be as well split for Stack Channel and Peripheral...
    OwnedSignal end_of_transfer; // BTB/ or FTB/

    // Inputs with I/O
    OwnedValue<uint16_t> new_pointer_address; // PAx/
    OwnedSignal apply_pointer_address;        // LOAD/

    OwnedValue<uint16_t> new_counter_value; // PRx/
    OwnedSignal apply_counter;              // STPC/ or STR/

    // Outputs with I/O
    OwnedValue<uint16_t> current_pointer_address; // Ax/

    // Inputs with Peripheral
    OwnedSignal direction;          // IN/OUT/
    OwnedSignal data_transfer;      // DT/ or DE/
    OwnedValue<uint8_t> input_data; // CSx/ or ESx/

    // Outputs with Peripheral
    OwnedSignal transfer_allowed;    // BT/ (or ACK/ ?)
    OwnedSignal output_strobe;       // STDO/
    OwnedValue<uint8_t> output_data; // CDx/ or SDx/

    // For debugging purposes
    struct DebugData
    {
        uint16_t memory_size;
        uint16_t data_pointer;
        uint16_t data_counter;
    };

    [[nodiscard]] DebugData get_debug_data() const;
    [[nodiscard]] uint8_t get_data_at(uint16_t address) const;

private:
    Scheduling::change_schedule_cb change_schedule;
    std::shared_ptr<Pluribus> pluribus;
    StackChannelCardConfiguration configuration;

    std::shared_ptr<IOCommunicator> io_communicator;

    std::vector<uint8_t> data;
    uint16_t data_pointer{};
    uint16_t data_counter{};

    void initialize_terminals();
    void initialize_io_communicator();
    void initialize_io_card_connections();
    void set_data_size();

    void on_t3(Edge edge);
    void on_apply_pointer_address(Edge edge);
    void on_apply_counter(Edge edge);
    void on_apply_pointer(Edge edge);

    void set_new_pointer(uint16_t new_pointer, Scheduling::counter_type time);

    uint8_t pop_data_to_bus(Scheduling::counter_type time);
    void push_data_from_bus(uint16_t address, Scheduling::counter_type time);
    uint8_t pop_data(Scheduling::counter_type time);
    void push_data(uint8_t out_data, Scheduling::counter_type time);

    [[nodiscard]] bool is_addressed(uint16_t address) const;
    void on_data_transfer(Edge edge);
    void stop_transfer_state(Scheduling::counter_type time);
};

#endif //MICRALN_STACKCHANNELCARD_H
