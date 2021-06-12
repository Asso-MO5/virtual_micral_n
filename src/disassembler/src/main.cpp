#include <file_utils/src/FileReader.h>
#include <mcs8/src/Disassemble8008.h>
#include <mcs8/src/MemoryView.h>
#include <misc_utils/src/ToHex.h>

#include <iostream>
#include <iomanip>
#include <sstream>

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
        auto [instruction, operand, size] = disassemble.get_extended(address);
        auto address_as_hex_string = utils::to_hex<std::uint16_t, 4>(address);

        std::cout << std::setfill(' ') << std::setw(8) << std::left << address_as_hex_string;

        std::stringstream bytes;
        for (std::uint16_t sub_address = address; sub_address < address + size; sub_address += 1)
        {
            bytes << std::setfill(' ') << std::setw(4) << std::left;
            bytes << utils::to_hex<std::uint8_t, 2>(rom_view.get(sub_address));
        }

        std::cout << std::setfill(' ') << std::setw(16) << std::left << bytes.str();

        std::cout << instruction.c_str();
        if (!operand.empty())
        {
            std::cout << " " << operand.c_str();
        }
        std::cout << "\n";

        if (instruction == "JMP" || instruction == "RET" || instruction == "REI")
        {
            std::cout << "\n";
        }

        address += size;
    }

    return 0;
}