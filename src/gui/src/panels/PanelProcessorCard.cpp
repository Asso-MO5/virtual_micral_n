#include "PanelProcessorCard.h"

#include "gui/src/widgets/PanelLed.h"

#include <devices/src/ProcessorCard.h>
#include <emulator/src/Simulator.h>
#include <imgui.h>

void display_processor_card_panel(const Simulator& simulator)
{
    const auto& scheduler = simulator.get_scheduler();
    const auto& debug_info = simulator.get_processor_card().get_debug_data();

    ImGui::Begin("Processor Card");

    ImGui::BeginGroup();
    ImGui::Text("Watchdog");
    display_led(debug_info.watchdog_on ? 1.f : 0.f, widgets::LedColor::GREEN);
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::End();
}
