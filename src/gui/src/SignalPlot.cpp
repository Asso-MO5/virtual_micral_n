#include "SignalPlot.h"

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui_internal.h>

namespace ImGui
{
    void PlotSignal(const PlotSignalConfig& config)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
        {
            return;
        }

        const ImGuiStyle& style = GImGui->Style;

        const ImRect frame_bounding_box(window->DC.CursorPos,
                                        window->DC.CursorPos + config.frame_size);
        const ImRect inner_bounding_box(frame_bounding_box.Min + style.FramePadding,
                                        frame_bounding_box.Max - style.FramePadding);
        const ImRect total_bounding_box = frame_bounding_box;
        ItemSize(total_bounding_box, style.FramePadding.y);
        if (!ItemAdd(total_bounding_box, 0, &frame_bounding_box))
        {
            return;
        }

        RenderFrame(frame_bounding_box.Min, frame_bounding_box.Max, GetColorU32(ImGuiCol_FrameBg),
                    true, style.FrameRounding);

        if (config.values.count > 0)
        {
            const ImU32 line_color = GetColorU32(ImGuiCol_PlotLines);

            const float x_min = config.values.x_series[config.values.offset];
            const float x_max =
                    config.values.x_series[config.values.offset + config.values.count - 1];

            const float inverse_scale_x = (x_max == x_min) ? 0.f : (1.0f / (x_max - x_min));
            const float inverse_scale_y = (config.scale.min == config.scale.max)
                                                  ? 0.0f
                                                  : (1.0f / (config.scale.max - config.scale.min));

            const float first_x_value = config.values.x_series[config.values.offset];
            const float first_y_value = config.values.y_series[config.values.offset];

            ImVec2 first_normalized_point = ImVec2(
                    0.f, 1.0f - ImSaturate((first_y_value - config.scale.min) * inverse_scale_y));

            const int end_index = config.values.offset + config.values.count;
            for (int value_index = config.values.offset; value_index < end_index; value_index++)
            {
                const float x_value = config.values.x_series[value_index] - first_x_value;
                const float y_value = config.values.y_series[value_index];
                const ImVec2 second_normalized_point =
                        ImVec2(ImSaturate(x_value * inverse_scale_x),
                               1.0f - ImSaturate((y_value - config.scale.min) * inverse_scale_y));

                ImVec2 first_position = ImLerp(inner_bounding_box.Min, inner_bounding_box.Max,
                                               first_normalized_point);
                ImVec2 second_position = ImLerp(inner_bounding_box.Min, inner_bounding_box.Max,
                                                second_normalized_point);

                window->DrawList->AddLine(first_position, second_position, line_color,
                                          config.line_thickness);
                first_normalized_point = second_normalized_point;
            }
        }
    }
} // namespace ImGui