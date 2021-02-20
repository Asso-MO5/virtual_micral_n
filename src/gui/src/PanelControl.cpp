#include "PanelControl.h"

#include "Simulator.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace
{
    enum ButtonType
    {
        TOGGLE,
        IMPULSE,
    };

    const ImVec4 INACTIVE_COLOR_HOVERED = ImVec4(0.78f, 0.78f, 0.78f, 1.0f);
    const ImVec4 INACTIVE_COLOR_NOT_HOVERED = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
    const ImU32 BORDER_COLOR = IM_COL32(255, 255, 255, 255);

    void display_control_button(const char* str_id, bool* value, ButtonType type)
    {
        const float height = ImGui::GetFrameHeight() * 1.50f;
        const float width = ImGui::GetFrameHeight();
        const float radius = width * 0.50f;

        // Getting the position before the invisible button
        const ImVec2 screen_position = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton(str_id, ImVec2(width, height));

        if (type == TOGGLE)
        {
            if (ImGui::IsItemClicked())
            {
                *value = !*value;
            }
        }
        else
        {
            if (ImGui::IsItemHovered() && ImGui::IsMouseDown(0))
            {
                *value = true;
            }
            else
            {
                *value = false;
            }
        }

        const ImVec4* style_color = ImGui::GetStyle().Colors;
        ImVec4 back_color{};
        if (ImGui::IsItemHovered())
        {
            back_color = *value ? style_color[ImGuiCol_ButtonActive] : INACTIVE_COLOR_HOVERED;
        }
        else
        {
            back_color = *value ? style_color[ImGuiCol_Button] : INACTIVE_COLOR_NOT_HOVERED;
        }

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        draw_list->AddRectFilled(screen_position,
                                 ImVec2(screen_position.x + width, screen_position.y + height),
                                 ImGui::GetColorU32(back_color), height * 0.5f);

        const auto circle_position = ImVec2(
                screen_position.x + radius,
                screen_position.y + radius + (*value ? 1.f : 0.f) * (height - radius * 2.0f));
        draw_list->AddCircleFilled(circle_position, radius - 1.5f, BORDER_COLOR);
    }
} // namespace

void PanelControl::display(Simulator& simulator)
{
    ImGui::Begin("Panel");

    auto interrupt_value_before = interrupt_value;

    ImGui::BeginGroup();
    ImGui::Text("INT");
    display_control_button("INT", &interrupt_value, IMPULSE);
    ImGui::EndGroup();

    if (interrupt_value != interrupt_value_before)
    {
        auto time = simulator.get_scheduler().get_counter();
        Edge::Front front = interrupt_value ? Edge::Front::RISING : Edge::Front::FALLING;
        simulator.get_interrupt_controller().wants_interrupt(Edge{front, time});
    }

    ImGui::SameLine();

    auto wait_value_before = wait_value;

    ImGui::BeginGroup();
    ImGui::Text("WAIT");
    display_control_button("WAIT", &wait_value, TOGGLE);
    ImGui::EndGroup();

    if (wait_value_before != wait_value)
    {
        auto time = simulator.get_scheduler().get_counter();
        Edge::Front front = wait_value ? Edge::Front::FALLING : Edge::Front::RISING;
        simulator.set_wait_line(Edge{front, time});
    }

    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Text("INPUT");

    for (auto& input_button : inputs)
    {
        display_control_button("B", &input_button, TOGGLE);
        ImGui::SameLine();
    }
    ImGui::EndGroup();

    ImGui::End();
}
