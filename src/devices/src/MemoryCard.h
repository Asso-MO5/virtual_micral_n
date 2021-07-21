#ifndef MICRALN_MEMORYCARD_H
#define MICRALN_MEMORYCARD_H

#include <emulation_core/src/Edge.h>
#include <emulation_core/src/Schedulable.h>
#include <i8008/src/Constants8008.h>

#include <memory>
#include <vector>

class DataOnMDBusHolder;
class MemoryPage;
class Pluribus;

struct MemoryCardConfiguration
{
    enum AddressingSize
    {
        Card2k,
        Card4k,
    };

    enum AccessType
    {
        RAM,
        ROM,
        ROM_RAM_256,
    };

    AddressingSize addressing_size;
    AccessType access_type;
    uint8_t selection_mask{};
};

class MemoryCard : public SchedulableImpl
{
public:
    using AddressingSize = MemoryCardConfiguration::AddressingSize;

    struct Config
    {
        Scheduling::change_schedule_cb change_schedule;
        std::shared_ptr<Pluribus> pluribus;
        MemoryCardConfiguration configuration;
    };

    explicit MemoryCard(const Config& config);
    ~MemoryCard() override;

    std::vector<std::shared_ptr<Schedulable>> get_sub_schedulables() override;

    void load_data(std::vector<uint8_t> data);

    void step() override;

    // For debugging purposes
    [[nodiscard]] uint16_t get_start_address() const;
    [[nodiscard]] uint16_t get_length() const;
    [[nodiscard]] uint8_t get_data_at(uint16_t address) const;

private:
    Scheduling::change_schedule_cb change_schedule;
    std::shared_ptr<Pluribus> pluribus;
    MemoryCardConfiguration configuration;

    std::vector<std::unique_ptr<MemoryPage>> page_readers;
    std::vector<std::unique_ptr<MemoryPage>> page_writers;

    std::shared_ptr<DataOnMDBusHolder> output_data_holder;

    std::vector<uint8_t> buffer;
    std::vector<uint8_t> masked_rom;

    void set_data_size();
    void create_memory_pages();

    void on_t2(Edge edge);
    void on_t3prime(Edge edge);
    void on_phase_2(Edge edge);

    bool is_addressed(uint16_t address);
    [[nodiscard]] uint8_t read_data(uint16_t address) const;
    void write_data(uint16_t address, uint8_t data);

    [[nodiscard]] std::tuple<uint16_t, size_t, uint16_t> get_local_address(uint16_t address) const;
    [[nodiscard]] AddressingSize get_addressing_size() const;
};

#endif //MICRALN_MEMORYCARD_H
