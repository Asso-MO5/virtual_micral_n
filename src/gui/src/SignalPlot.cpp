#include "SignalPlot.h"

#include <imgui_internal.h>

#include "Plot.h"

namespace ImGui
{
    void PlotSignal(const PlotSignalConfig& config)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
        {
            return;
        }

        const auto [displayed, bb_min, bb_max] = render_plot_frame(config.frame_size);
        if (!displayed)
        {
            return;
        }

        ImRect inner_bounding_box{bb_min, bb_max};

        if (config.values.count > 0)
        {
            const ImU32 line_color = GetColorU32(ImGuiCol_PlotLines);

            const auto [x_min, x_max, inverse_scale_x, inverse_scale_y] =
                    get_x_min_max_scales(config.values, config.scale);

            auto [first_valid_index, first_x_value, first_y_value] =
                    get_first_values(config.values, x_min);

            ImVec2 first_normalized_point = ImVec2(
                    ImSaturate(static_cast<float>((first_x_value - x_min) * inverse_scale_x)),
                    1.0f - ImSaturate(static_cast<float>((first_y_value - config.scale.y_min) *
                                                         inverse_scale_y)));

            // Plot the line before the first point if needed
            if (first_x_value > x_min)
            {
                ImVec2 first_position = ImLerp(inner_bounding_box.Min, inner_bounding_box.Max,
                                               first_normalized_point);
                ImVec2 starting_position = first_position;
                starting_position.x = inner_bounding_box.Min.x;

                window->DrawList->AddLine(starting_position, first_position, line_color,
                                          config.line_thickness);
            }

            // Plot the values
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

            // Extends the last value if needed
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