#include "PanelDiskController.h"

#include <devices/src/DiskControllerCard.h>

#include <gui/src/widgets/PanelLed.h>
#include <imgui.h>

void display_disk_controller(const DiskControllerCard& disk_controller)
{
    ImGui::Begin("Disk Controller");

    const auto& status = disk_controller.get_debug_data();

    ImGui::Text("Track: $%002x", status.track);
    ImGui::Text("Sector: $%002x", status.sector);

    ImGui::BeginGroup();
    ImGui::Text("Read");
    display_led(status.sending_to_channel ? 1.f : 0.f, widgets::LedColor::GREEN);
    ImGui::EndGroup();

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    ImGui::BeginGroup();
    ImGui::Text("Write");
    display_led(0.f, widgets::LedColor::RED);
    ImGui::EndGroup();

    ImGui::End();
}
