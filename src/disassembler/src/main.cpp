#include "OutputLine.h"

#include <file_utils/src/FileReader.h>
#include <i8008/src/Disassemble8008.h>
#include <i8008/src/MemoryView.h>

#include <CLI/CLI.hpp>
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

void disassemble_file(const std::string& filename, std::uint16_t start_address)
{
    const auto rom = FileReader{filename};

    const RomView rom_view{rom.data};
    Disassemble8008 disassemble{rom_view};

    std::uint16_t address = 0;

    while (address < rom_view.size())
    {
        OutputLine line(disassemble, rom_view, address, start_address);
        std::cout << line << "\n";

        if (line.is_end_of_block_instruction())
        {
            std::cout << "\n";
        }

        address += line.get_instruction_size();
    }
}

int main(int argc, char** argv)
{
    CLI::App disassembler_app{"Micral N 8008 disassembler"};

    std::string input_file;
    disassembler_app
            .add_option("-i,--input,input", input_file, "Input binary file to disassemble.")
            ->required()
            ->check(CLI::ExistingFile);

    std::uint16_t origin_address = 0;
    disassembler_app.add_option("--orig", origin_address, "Address the ROM starts at.")
            ->default_val(0)
            ->check(CLI::Number)
            ->check(CLI::Range(0, 16383));

    try
    {
        disassembler_app.parse(argc, argv);
    }
    catch (const CLI::ParseError& e)
    {
        return disassembler_app.exit(e);
    }

    disassemble_file(input_file, origin_address);

    return 0;
}
