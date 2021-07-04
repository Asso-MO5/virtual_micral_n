#include "PanelDiskController.h"

#include <devices/src/DiskControllerCard.h>

#include <gui/src/widgets/PanelLed.h>
#include <imgui.h>

void display_disk_controller(const DiskControllerCard& simulator)
{
    ImGui::Begin("Disk Controller");

    ImGui::Text("Track: $%002x", 0);
    ImGui::Text("Sector: $%002x", 0);

    ImGui::BeginGroup();
    ImGui::Text("Read");
    display_led(0.f, widgets::LedColor::GREEN);
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
