#include "OutputLine.h"

#include <file_utils/src/FileReader.h>
#include <mcs8/src/Disassemble8008.h>
#include <mcs8/src/MemoryView.h>

#include <iostream>

namespace
{
    class RomView : public MemoryView
    {
    public:
        using data_container_type = std::vector<std::uint8_t>;
        explicit RomView(const data_container_type& data) : data{data} {}

        [[nodiscard]] std::uint8_t get(std::uint16_t address) const override
        {
            return data[address];
        }
        [[nodiscard]] std::size_t size() const override { return data.size(); }

    private:
        const data_container_type& data;
    };
}

int main()
{
    const auto rom = FileReader{"data/MIC_1_EPROM_CARTE_MEM_4K.BIN"};

    const RomView rom_view{rom.data};
    Disassemble8008 disassemble{rom_view};

    std::uint16_t address = 0;

    while (address < rom_view.size())
    {
        OutputLine line(disassemble, rom_view, address);
        std::cout << line << "\n";

        if (line.is_end_of_block_instruction())
        {
            std::cout << "\n";
        }

        address += line.get_instruction_size();
    }

    return 0;
}
