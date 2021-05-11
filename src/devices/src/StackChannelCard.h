#ifndef MICRALN_STACKCHANNELCARD_H
#define MICRALN_STACKCHANNELCARD_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/OwnedSignal.h>
#include <emulation_core/src/OwnedValue.h>
#include <emulation_core/src/Schedulable.h>

#include <memory>
#include <vector>

class Pluribus;
class DataOnMDBusHolder;
class IOCard;

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

    // An optional connection to an io card for controls.
    // At the moment, only one card is used for all I/O controls.
    std::shared_ptr<IOCard> io_card{};

    // Inputs with I/O
    // STPC/ or STR/ for Apply, PRx/ for value
    uint8_t new_counter_terminal; // The terminal on the I/O card
};

class StackChannelCard : public SchedulableImpl
{
public:
    struct Config
    {
        SignalReceiver& scheduler;
        std::shared_ptr<Pluribus> pluribus;
        StackChannelCardConfiguration configuration;
    };

    explicit StackChannelCard(const Config& config);
    ~StackChannelCard() override;

    void step() override;

    // Unknown direction with I/O
    OwnedSignal transfer_cycle; // TC/

    // Output with... I/O or Peripheral, depending on the paragraph
    OwnedSignal end_of_transfer; // BTB/ or FTB/

    // Inputs with I/O
    OwnedValue<uint16_t> new_pointer_address; // PAx/
    OwnedSignal apply_pointer_address;        // LOAD/

    // Outputs with I/O
    OwnedValue<uint16_t> current_pointer_address; // Ax/

    // Inputs with Peripheral
    OwnedSignal direction;          // IN/OUT/
    OwnedSignal data_transfer;      // DT/ or DE/
    OwnedValue<uint8_t> input_data; // CSx/ or ESx/

    // Outputs with Peripheral
    OwnedSignal in_transfer;         // BT/ (or ACK/ ?)
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
    SignalReceiver& scheduler;
    std::shared_ptr<Pluribus> pluribus;
    StackChannelCardConfiguration configuration;

    std::unique_ptr<DataOnMDBusHolder> output_data_holder;

    std::vector<uint8_t> data;
    uint16_t data_pointer{};
    uint16_t data_counter{};

    void set_data_size();

    void on_t2(Edge edge);
    void on_t3(Edge edge);
    void on_apply_pointer_address(Edge edge);
    void on_apply_counter(Edge edge);

    uint8_t pop_data_to_bus(Scheduling::counter_type time);
    void push_data_from_bus(uint16_t address, Scheduling::counter_type time);
    uint8_t pop_data(Scheduling::counter_type time);
    void push_data(uint8_t out_data, Scheduling::counter_type time);

    [[nodiscard]] bool is_addressed(uint16_t address) const;
    void initialize_terminals();
    void on_data_transfer(Edge edge);
    void initialize_io_card_connections();
};

#endif //MICRALN_STACKCHANNELCARD_H
