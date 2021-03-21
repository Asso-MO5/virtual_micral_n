#include "ValuePlot.h"

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui_internal.h>

#include "Plot.h"

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
    }

} // namespace ImGui
