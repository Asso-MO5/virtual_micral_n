#include "Disassemble8008.h"

#include <MemoryView.h>

#include "gmock/gmock.h"

using namespace testing;

class OwningMemoryView : public MemoryView
{
public:
    explicit OwningMemoryView(std::span<uint8_t> memory) : memory{begin(memory), end(memory)} {}

    [[nodiscard]] uint8_t get(std::uint16_t address) const override { return memory[address]; }
    [[nodiscard]] size_t size() const override { return memory.size(); }

private:
    std::vector<std::uint8_t> memory;
};

std::uint8_t MemoryView::get(std::uint16_t address) const { return 0; }

TEST(Disassemble8008, needs_to_be_associated_to_a_data_view)
{
    std::vector<std::uint8_t> data{0x00};
    OwningMemoryView memory_view{data};
    Disassemble8008 disassemble{memory_view};

    auto [instruction, size] = disassemble.get(0x0000);

    ASSERT_THAT(instruction, Eq("HLT"));
}

TEST(Disassemble8008, decodes_immediate_8bit_values)
{
    std::vector<std::uint8_t> data{0x2e, 0xf0};
    OwningMemoryView memory_view{data};
    Disassemble8008 disassemble{memory_view};

    auto [instruction, size] = disassemble.get(0x0000);

    ASSERT_THAT(instruction, Eq("LHI $f0"));
}

TEST(Disassemble8008, decodes_immediate_16bit_values)
{
    std::vector<std::uint8_t> data{0x44, 0x00, 0x20};
    OwningMemoryView memory_view{data};
    Disassemble8008 disassemble{memory_view};

    auto [instruction, size] = disassemble.get(0x0000);

    ASSERT_THAT(instruction, Eq("JMP $2000"));
}

TEST(Disassemble8008, decodes_rst)
{
    std::vector<std::uint8_t> data{0x25};
    OwningMemoryView memory_view{data};
    Disassemble8008 disassemble{memory_view};

    auto [instruction, size] = disassemble.get(0x0000);

    ASSERT_THAT(instruction, Eq("RST $20"));
}

TEST(Disassemble8008, decodes_inp)
{
    std::vector<std::uint8_t> data{0x43};
    OwningMemoryView memory_view{data};
    Disassemble8008 disassemble{memory_view};

    auto [instruction, size] = disassemble.get(0x0000);

    ASSERT_THAT(instruction, Eq("INP $1"));
}

TEST(Disassemble8008, decodes_out)
{
    std::vector<std::uint8_t> data{0x7f};
    OwningMemoryView memory_view{data};
    Disassemble8008 disassemble{memory_view};

    auto [instruction, size] = disassemble.get(0x0000);

    ASSERT_THAT(instruction, Eq("OUT $17"));
}

TEST(Disassemble8008, decodes_a_series_of_instructions)
{
    std::vector<std::uint8_t> data{0x00, 0x2e, 0xf0, 0x44, 0x00, 0x20, 0x25};
    OwningMemoryView memory_view{data};
    Disassemble8008 disassemble{memory_view};

    std::vector<std::string> results;

    size_t address = 0;
    while (address < data.size()) {
        auto [instruction, size] = disassemble.get(address);

        results.push_back(instruction);
        address += size;
    }

    ASSERT_THAT(results, ElementsAre("HLT", "LHI $f0", "JMP $2000", "RST $20"));
}
