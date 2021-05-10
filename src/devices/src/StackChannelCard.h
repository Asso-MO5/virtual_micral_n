#ifndef MICRALN_STACKCHANNELCARD_H
#define MICRALN_STACKCHANNELCARD_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>

#include <memory>
#include <vector>

class Pluribus;
class DataOnMDBusHolder;

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
        SignalReceiver& scheduler;
        std::shared_ptr<Pluribus> pluribus;
        StackChannelCardConfiguration configuration;
    };

    explicit StackChannelCard(const Config& config);
    ~StackChannelCard() override;

    void step() override;

    // For debugging purposes
    struct DebugData
    {
        uint16_t memory_size;
        uint16_t data_pointer;
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

    void set_data_size();
    void on_t2(Edge edge);
    void on_t3(Edge edge);
    uint8_t pop_data();
    void push_data(uint16_t address);
    [[nodiscard]] bool is_addressed(uint16_t address) const;
};

#endif //MICRALN_STACKCHANNELCARD_H
