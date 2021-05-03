#ifndef MICRALN_MEMORYCARD_H
#define MICRALN_MEMORYCARD_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>
#include <mcs8/src/Constants8008.h>

#include <memory>
#include <vector>

class Pluribus;
class DataOnMDBusHolder;

namespace MemoryConstants
{
    constexpr const size_t SELECTION_BIT_COUNT = 3;
    constexpr const size_t MAXIMUM_PAGES = 8;
} // namespace MemoryConstants

struct MemoryCardConfiguration
{
    enum AddressingSize
    {
        Card2k,
        Card4k,
    };

    AddressingSize addressing_size;
    std::array<bool, MemoryConstants::MAXIMUM_PAGES> writable_page{};
    std::array<bool, MemoryConstants::SELECTION_BIT_COUNT> selection_mask{};
};

class MemoryCard : public SchedulableImpl
{
public:
    using AddressingSize = MemoryCardConfiguration::AddressingSize;

    struct Config
    {
        SignalReceiver& scheduler;
        std::shared_ptr<Pluribus> pluribus;
        MemoryCardConfiguration configuration;
    };

    explicit MemoryCard(const Config& config);
    ~MemoryCard() override;

    void load_data(std::vector<uint8_t> data);

    void step() override;

    // For debugging purposes
    [[nodiscard]] uint16_t get_start_address() const;
    [[nodiscard]] uint16_t get_length() const;
    [[nodiscard]] uint8_t get_data_at(uint16_t address) const;

private:
    SignalReceiver& scheduler;
    std::shared_ptr<Pluribus> pluribus;
    MemoryCardConfiguration configuration;

    std::unique_ptr<DataOnMDBusHolder> output_data_holder;

    std::vector<uint8_t> data;

    void on_t2(Edge edge);
    void on_t3(Edge edge);
    void on_t3prime(Edge edge);

    bool is_addressed(uint16_t address);
    [[nodiscard]] uint8_t get_data(uint16_t address) const;
    void set_data_size();

    [[nodiscard]] AddressingSize get_addressing_size() const;
};

MemoryCardConfiguration get_rom_2k_configuration(bool s13, bool s12, bool s11);
MemoryCardConfiguration get_ram_2k_configuration(bool s13, bool s12, bool s11);

#endif //MICRALN_MEMORYCARD_H
