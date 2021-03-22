#ifndef MICRALN_PLOT_H
#define MICRALN_PLOT_H

#include "PlotStructures.h"

#include <imgui.h>
#include <tuple>

namespace ImGui
{
    std::tuple<bool, ImVec2, ImVec2> render_plot_frame(const ImVec2& frame_size);

    template<typename ValueType>
    std::tuple<double, double, double, double>
    get_x_min_max_scales(const PlotValues<ValueType> values, const PlotScale& scale)
    {
        const double x_min = scale.x_scaled ? scale.x_min : values.x_series[values.offset];
        const double x_max =
                scale.x_scaled ? scale.x_max : values.x_series[values.offset + values.count - 1];

        const double inverse_scale_x = (x_max == x_min) ? 0.0 : (1.0 / (x_max - x_min));
        const double inverse_scale_y =
                (scale.y_min == scale.y_max) ? 0.0 : (1.0 / (scale.y_max - scale.y_min));

        return {x_min, x_max, inverse_scale_x, inverse_scale_y};
    }

} // namespace ImGui

#endif //MICRALN_PLOT_H
