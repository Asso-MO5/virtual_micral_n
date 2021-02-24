#include "PanelLed.h"

#include <imgui.h>

namespace widgets
{
    const ImVec4 LED_GREEN_ON = ImVec4(0.0f, 1.0f, 0.f, 1.0f);

    void display_led(bool value, LedColor color_type)
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const ImVec2 screen_position = ImGui::GetCursorScreenPos();
        const float radius = 5.f;
        const auto circle_position = ImVec2(screen_position.x + radius, screen_position.y + radius);

        assert(color_type == GREEN); // Only color at now
        auto color = LED_GREEN_ON;

        if (!value)
        {
            color.x *= 0.2f;
            color.y *= 0.2f;
            color.z *= 0.2f;
        }
        draw_list->AddCircleFilled(circle_position, radius, ImGui::GetColorU32(color));
    }

}


