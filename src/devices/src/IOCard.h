#ifndef MICRALN_IOCARD_H
#define MICRALN_IOCARD_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>

#include <memory>
#include <vector>

class Pluribus;
class DataOnMDBusHolder;

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
    uint8_t address_mask{};
};

class IOCard : public SchedulableImpl
{
public:
    struct Config
    {
        SignalReceiver& scheduler;
        std::shared_ptr<Pluribus> pluribus;
        IOCardConfiguration configuration;
    };

    explicit IOCard(const Config& config);
    ~IOCard() override;

    void step() override;

private:
    SignalReceiver& scheduler;
    std::shared_ptr<Pluribus> pluribus;
    IOCardConfiguration configuration;

    std::unique_ptr<DataOnMDBusHolder> output_data_holder;

    void on_t2(Edge edge);
    void on_t3(Edge edge);

    [[nodiscard]] bool is_addressed(uint16_t address) const;
};

#endif //MICRALN_IOCARD_H
