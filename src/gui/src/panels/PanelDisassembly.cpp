#include "PanelDisassembly.h"

#include <devices/src/CPU8008.h>
#include <devices/src/ProcessorCard.h>
#include <emulator/src/Simulator.h>
#include <i8008/src/Disassemble8008.h>

#include <cstdint>
#include <imgui.h>

void display_disassembly_panel(Simulator& simulator, Disassemble8008& disassemble)
{
    static uint16_t starting_disassembly_address = 0x0000;

    ImGui::Begin("Disassembly");
    const bool scroll_to_position = ImGui::Button("Center on PC");

    ImGui::BeginChild("Content");

    uint16_t latest_pci =
            simulator.get_processor_card().get_cpu().get_debug_data().latest_emitted_pci;

    uint16_t address = starting_disassembly_address;
    const uint16_t end_address = address + simulator.get_memory_view().size();
    while (address < end_address)
    {
        auto [instruction, size] = disassemble.get_as_string(address);
        const auto is_current_pc = address == latest_pci;

        ImGui::Text("%s $%04x:  %s", is_current_pc ? ">" : " ", address, instruction.c_str());
        address += size;

        if (scroll_to_position && is_current_pc)
        {
            ImGui::SetScrollHereY();
        }
    }

    ImGui::EndChild();

    ImGui::End();
}
