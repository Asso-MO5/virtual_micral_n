#include "PanelStackChannelCard.h"

#include "gui/src/widgets/PanelMemoryDisplay.h"

#include <devices/src/StackChannelCard.h>
#include <emulator/src/Simulator.h>

#include <imgui.h>
#include <sstream>

void display_stack_channel_card_panel(const Simulator& simulator, uint8_t card_number)
{
    const auto& scheduler = simulator.get_scheduler();
    const auto& stack_channel_card = simulator.get_stack_channel_card(card_number);

    std::stringstream title_stream;
    title_stream << "Stack Channel Card " << static_cast<uint32_t>(card_number);

    const auto debug_data = stack_channel_card.get_debug_data();

    ImGui::Begin(title_stream.str().c_str());

    ImGui::Text("Pointer: %04x", debug_data.data_pointer);
    ImGui::Text("Counter: %04x", debug_data.data_counter);

    auto memory_parameters =
            MemoryDisplayParameters{.start_address = 0x0000,
                                    .highlighted_memory = debug_data.data_pointer,
                                    .memory_size = debug_data.memory_size,
                                    .memory_access_function = [&stack_channel_card](auto address) {
                                        return stack_channel_card.get_data_at(address);
                                    }};

    memory_display<16>("Memory", memory_parameters);

    ImGui::End();
}
