#include "ControllerWidget.h"

#include "imgui.h"

ControllerWidget::ControllerWidget(bool& running) : running(running) {}

void ControllerWidget::update()
{
    ImGui::Begin("Control");

    if (running)
    {
        ImGui::Text("Running");
    }
    else
    {
        ImGui::Text("Paused");
    }

    if (step && running) {
        step = false;
        running = false;
    }

    if (ImGui::Button("Run"))
    {
        running = true;
    }
    if (ImGui::Button("Pause"))
    {
        running = false;
    }
    if (ImGui::Button("Step one frame"))
    {
        // At the moment, the Step button advances one full render frame.
        // Probably not what we want.
        running = true;
        step = true;
    }

    ImGui::End();
}
