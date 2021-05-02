#include "PanelDisassembly.h"

#include <devices/src/CPU8008.h>
#include <devices/src/ProcessorCard.h>
#include <emulator/src/Simulator.h>
#include <mcs8/src/Disassemble8008.h>

#include <cstdint>
#include <imgui.h>

void display_disassembly_panel(Simulator& simulator, Disassemble8008& disassemble)
{
    static uint16_t starting_disassembly_address = 0x0000;
    ImGui::Begin("Disassembly");

    uint16_t latest_pci =
            simulator.get_processor_card().get_cpu().get_debug_data().latest_emitted_pci;

    uint16_t address = starting_disassembly_address;
    const uint16_t end_address = address + simulator.get_memory_view().size();
    while (address < end_address)
    {
        auto [instruction, size] = disassemble.get(address);

        ImGui::Text("%s $%04x:  %s", (address == latest_pci) ? ">" : " ", address,
                    instruction.c_str());
        address += size;
    }

    ImGui::End();
}
