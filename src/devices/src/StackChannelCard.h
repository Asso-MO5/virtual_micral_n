#ifndef MICRALN_STACKCHANNELCARD_H
#define MICRALN_STACKCHANNELCARD_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>
#include <mcs8/src/Constants8008.h>

#include <memory>
#include <vector>

class Pluribus;

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
    ~StackChannelCard() override = default;

    void step() override;

    // For debugging purposes
    [[nodiscard]] uint16_t get_size() const;
    [[nodiscard]] uint8_t get_data_at(uint16_t address) const;

private:
    SignalReceiver& scheduler;
    std::shared_ptr<Pluribus> pluribus;
    StackChannelCardConfiguration configuration;

    std::vector<uint8_t> data;
    size_t data_pointer{};

    uint8_t latched_data{};
    bool is_emitting_data{false};

    void set_data_size();
    void on_t2(Edge edge);
    void on_t3(Edge edge);
    void pop_data();
    void push_data(uint16_t address);
    [[nodiscard]] bool is_addressed(uint16_t address) const;
};

#endif //MICRALN_STACKCHANNELCARD_H
