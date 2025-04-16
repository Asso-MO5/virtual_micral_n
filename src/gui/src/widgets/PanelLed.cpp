#include "PanelLed.h"

#include <algorithm>

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <imgui.h>
#include <imgui_internal.h>

namespace widgets
{
    using namespace ImGui;

    const ImVec4 LED_GREEN_ON = ImVec4(0.0f, 1.0f, 0.f, 1.0f);
    const ImVec4 LED_RED_ON = ImVec4(1.0f, 0.0f, 0.f, 1.0f);

    void display_led(float intensity, LedColor color_type)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
        {
            return;
        }

        const float radius = 10.f;

        const ImVec2 screen_position = ImGui::GetCursorScreenPos();
        const ImGuiStyle& style = GImGui->Style;

        const float size = radius * 2.f; //GetFrameHeight();
        const ImRect total_bounding_box(screen_position, screen_position + ImVec2(size, size));
        ItemSize(total_bounding_box, 0);
        if (!ItemAdd(total_bounding_box, 0))
        {
            return;
        }

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const auto circle_position = ImVec2(screen_position.x + radius, screen_position.y + radius);

        ImVec4 color;
        switch (color_type)
        {
            case GREEN:
                color = LED_GREEN_ON;
                break;
            case RED:
                color = LED_RED_ON;
                break;
        }

        auto color_modifier =
                std::max(intensity, 0.2f); // LED not lighted has still a bit of color.

        color.x *= color_modifier;
        color.y *= color_modifier;
        color.z *= color_modifier;

        draw_list->AddCircleFilled(circle_position, radius, ImGui::GetColorU32(color));
    }

} // namespace widgets
