#include "PanelMemory.h"

#include "gui/src/widgets/PanelMemoryDisplay.h"

#include <devices/src/CPU8008.h>
#include <devices/src/ProcessorCard.h>
#include <emulator/src/Simulator.h>
#include <i8008/src/Disassemble8008.h>

#include <cstdint>
#include <imgui.h>

void display_memory_panel(Simulator& simulator)
{
    static uint16_t starting_disassembly_address = 0x0000;
    ImGui::Begin("Memory");

    // TODO: Better would be to look at the control bus for debug data on latest fetch and latest write.
    uint16_t latest_pci =
            simulator.get_processor_card().get_cpu().get_debug_data().latest_emitted_pci;

    auto memory_parameters =
            MemoryDisplayParameters{.start_address = starting_disassembly_address,
                                    .highlighted_memory = latest_pci,
                                    .memory_size = simulator.get_memory_view().size(),
                                    .memory_access_function = [&simulator](auto address) {
                                        return simulator.get_memory_view().get(address);
                                    }};

    memory_display<16>("Memory", memory_parameters);

    ImGui::End();
}
