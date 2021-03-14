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

            const double x_min = config.scale.x_scaled
                                         ? config.scale.x_min
                                         : config.values.x_series[config.values.offset];
            const double x_max =
                    config.scale.x_scaled
                            ? config.scale.x_max
                            : config.values
                                      .x_series[config.values.offset + config.values.count - 1];

            const double inverse_scale_x = (x_max == x_min) ? 0.0 : (1.0 / (x_max - x_min));
            const double inverse_scale_y =
                    (config.scale.y_min == config.scale.y_max)
                            ? 0.0
                            : (1.0 / (config.scale.y_max - config.scale.y_min));

            auto first_valid_index = config.values.offset;
            double first_x_value = config.values.x_series[first_valid_index];
            double first_y_value = config.values.y_series[first_valid_index];

            while ((first_x_value < x_min) && (first_valid_index < config.values.count - 1))
            {
                first_valid_index += 1;
                first_x_value = config.values.x_series[first_valid_index];
                first_y_value = config.values.y_series[first_valid_index];
            }

            ImVec2 first_normalized_point = ImVec2(
                    ImSaturate(static_cast<float>((first_x_value - x_min) * inverse_scale_x)),
                    1.0f - ImSaturate(static_cast<float>((first_y_value - config.scale.y_min) *
                                                         inverse_scale_y)));

            if (first_x_value > x_min)
            {
                ImVec2 first_position = ImLerp(inner_bounding_box.Min, inner_bounding_box.Max,
                                               first_normalized_point);
                ImVec2 starting_position = first_position;
                starting_position.x = inner_bounding_box.Min.x;

                window->DrawList->AddLine(starting_position, first_position, line_color,
                                          config.line_thickness);
            }

            ImVec2 second_position;

            const int end_index = config.values.offset + config.values.count;
            for (int value_index = first_valid_index; value_index < end_index; value_index++)
            {
                const double x_value = config.values.x_series[value_index] - x_min;
                const double y_value = config.values.y_series[value_index];
                const ImVec2 second_normalized_point =
                        ImVec2(ImSaturate(static_cast<float>(x_value * inverse_scale_x)),
                               1.0f - ImSaturate(static_cast<float>((y_value - config.scale.y_min) *
                                                                    inverse_scale_y)));

                ImVec2 first_position = ImLerp(inner_bounding_box.Min, inner_bounding_box.Max,
                                               first_normalized_point);
                second_position = ImLerp(inner_bounding_box.Min, inner_bounding_box.Max,
                                         second_normalized_point);

                window->DrawList->AddLine(first_position, second_position, line_color,
                                          config.line_thickness);
                first_normalized_point = second_normalized_point;
            }

            if (config.values.x_series[end_index - 1] != x_max)
            {
                ImVec2 last_position = second_position;
                last_position.x = inner_bounding_box.Max.x;
                window->DrawList->AddLine(second_position, last_position, line_color,
                                          config.line_thickness);
            }
        }
    }
} // namespace ImGui