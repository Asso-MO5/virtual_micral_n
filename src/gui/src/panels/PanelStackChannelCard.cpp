#include "PanelStackChannelCard.h"

#include "gui/src/widgets/PanelLed.h"

#include <emulator/src/Simulator.h>
#include <imgui.h>
#include <sstream>

void display_stack_channel_card_panel(const Simulator& simulator, uint8_t card_number)
{
    const auto& scheduler = simulator.get_scheduler();
    const auto& debug_info = simulator.get_stack_channel_card(card_number);

    std::stringstream title_stream;
    title_stream << "Stack Channel Card " << static_cast<uint32_t>(card_number);

    ImGui::Begin(title_stream.str().c_str());

    ImGui::BeginGroup();
    ImGui::Text("Memory");
    // display_led(debug_info.watchdog_on ? 1.f : 0.f, widgets::LedColor::GREEN);
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::End();
}
