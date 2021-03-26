#include "ValuePlot.h"

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui_internal.h>

#include <iomanip>
#include <sstream>
#include <string>

#include "Plot.h"

namespace
{
    template<typename IntType>
    std::string to_hex(IntType value, size_t padding)
    {
        std::stringstream hex_value;
        hex_value << " $" << std::setfill('0') << std::setw(padding) << std::hex
                  << static_cast<int>(value);

        return hex_value.str();
    }

} // namespace
namespace ImGui
{
    void PlotValue(const ImGui::PlotValueConfig& config)
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

        const ImU32 line_color = GetColorU32(ImGuiCol_PlotLines);
        ImRect inner_bounding_box{bb_min, bb_max};

        if (config.data_values.count > 0)
        {
            const size_t padding = config.bus_width <= 8 ? 2 : 4;
            const auto [x_min, x_max, inverse_scale_x, inverse_scale_y] =
                    get_x_min_max_scales(config.data_values, config.scale);

            auto [first_valid_index, first_x_value, first_y_value] =
                    get_first_values(config.data_values, x_min);

            const int end_index = config.data_values.offset + config.data_values.count;
            for (int value_index = first_valid_index; value_index < end_index; value_index++)
            {
                const double x_value = config.data_values.x_series[value_index] - x_min;
                const double y_value = config.data_values.y_series[value_index];

                const ImVec2 normalized_position =
                        ImVec2(ImSaturate(static_cast<float>(x_value * inverse_scale_x)), 0.2);

                auto value_position =
                        ImLerp(inner_bounding_box.Min, inner_bounding_box.Max, normalized_position);

                auto value = to_hex(y_value, padding);

                auto* text_start = value.c_str();
                auto* text_end = text_start + value.size();

                RenderText(value_position, text_start, text_end, false);

                const ImVec2 line_up = ImVec2{normalized_position.x, 0.0};
                const ImVec2 line_down = ImVec2{normalized_position.x, 0.99};

                const auto line_up_position =
                        ImLerp(inner_bounding_box.Min, inner_bounding_box.Max, line_up);
                const auto line_down_position =
                        ImLerp(inner_bounding_box.Min, inner_bounding_box.Max, line_down);

                window->DrawList->AddLine(line_up_position, line_down_position, line_color,
                                          config.line_thickness);
            }
        }

        if (config.owner_values.count > 0)
        {
            const auto [x_min, x_max, inverse_scale_x, inverse_scale_y] =
                    get_x_min_max_scales(config.owner_values, config.scale);

            auto [first_valid_index, first_x_value, first_y_value] =
                    get_first_values(config.owner_values, x_min);

            uint32_t previous_owner = first_y_value;
            double previous_x = 0;

            if (first_y_value == 0)
            {
                const ImVec2 first_position_left = ImVec2(0.0, 0.0);
                auto left_position =
                        ImLerp(inner_bounding_box.Min, inner_bounding_box.Max, first_position_left);

                const ImVec2 first_position_right = ImVec2(
                        ImSaturate(static_cast<float>((first_x_value - x_min) * inverse_scale_x)),
                        1.0);

                auto right_position = ImLerp(inner_bounding_box.Min, inner_bounding_box.Max,
                                             first_position_right);

                window->DrawList->AddRect(left_position, right_position, line_color);
            }

            const int end_index = config.owner_values.offset + config.owner_values.count;
            for (int value_index = first_valid_index; value_index < end_index; value_index++)
            {
                const double x_value = config.owner_values.x_series[value_index] - x_min;
                const uint32_t owner = config.owner_values.y_series[value_index];

                if (previous_owner != 0 && owner == 0)
                {
                    const ImVec2 normalized_position_left = ImVec2(
                            ImSaturate(static_cast<float>(previous_x * inverse_scale_x)), 0.0);

                    auto left_position = ImLerp(inner_bounding_box.Min, inner_bounding_box.Max,
                                                normalized_position_left);

                    const ImVec2 normalized_position_right =
                            ImVec2(ImSaturate(static_cast<float>(x_value * inverse_scale_x)), 1.0);

                    auto right_position = ImLerp(inner_bounding_box.Min, inner_bounding_box.Max,
                                                 normalized_position_right);

                    window->DrawList->AddRect(left_position, right_position, line_color);
                }

                previous_owner = owner;
                previous_x = x_value;
            }
        }
    }

} // namespace ImGui
