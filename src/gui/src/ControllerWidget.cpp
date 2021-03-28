#include "ControllerWidget.h"

#include "imgui.h"

void ControllerWidget::update()
{
    ImGui::Begin("Control");

    if (state == RUNNING)
    {
        ImGui::Text("Running");
    }
    else
    {
        ImGui::Text("Paused");
    }

    if (state != RUNNING && state != RUNNING_SLOW && state != PAUSED)
    {
        state = PAUSED;
    }

    if (ImGui::Button("Run"))
    {
        state = RUNNING;
    }
    if (ImGui::Button("Run Slow"))
    {
        state = RUNNING_SLOW;
    }
    if (ImGui::Button("Pause"))
    {
        state = PAUSED;
    }
    if (ImGui::Button("Step one clock"))
    {
        state = STEP_ONE_CLOCK;
    }
    if (ImGui::Button("Step one state"))
    {
        state = STEP_ONE_STATE;
    }
    if (ImGui::Button("Step one frame"))
    {
        state = STEP_ONE_FRAME;
    }

    if (ImGui::Button("Step one instruction"))
    {
        state = STEP_ONE_INSTRUCTION;
    }

    ImGui::End();
}

SimulationRunType ControllerWidget::get_state() const { return state; }
