#include "PanelMemory.h"

#include "Simulator.h"

#include <array>
#include <cstdint>
#include <devices/src/CPU8008.h>
#include <devices/src/Disassemble8008.h>
#include <imgui.h>

void display_memory_panel(Simulator& simulator)
{
    static uint16_t starting_disassembly_address = 0x0000;
    ImGui::Begin("Memory");

    // TODO: Better would be to look at the control bus for debug data on latest fetch and latest write.
    uint16_t latest_pci = simulator.get_cpu().get_debug_data().latest_emitted_pci;

    uint16_t address = starting_disassembly_address;
    const uint16_t end_address = address + simulator.get_memory_view().size();

    const size_t data_width = 16;

    std::array<char, data_width + 1> char_view{};
    char_view.back() = 0;
    uint16_t memory_line = 0;

    while (address < end_address)
    {
        const uint8_t value = simulator.get_memory_view().get(address);

        if (address % data_width == 0)
        {
            ImGui::Text("$%004x: ", address);
            memory_line = address;
        }

        auto char_equivalent = (value < 32 || value > 127) ? '.' : value;
        char_view[address - memory_line] = char_equivalent;

        ImGui::SameLine();

        if (address == latest_pci)
        {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%002x ", value);
        }
        else
        {
            ImGui::Text("%002x ", value);
        }

        address += 1;

        if (address % data_width == 0)
        {
            ImGui::SameLine();
            ImGui::Text("  %s", char_view.data());
        }
    }

    ImGui::End();
}
