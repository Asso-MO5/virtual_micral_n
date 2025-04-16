#ifndef MICRALN_SIGNALPLOT_H
#define MICRALN_SIGNALPLOT_H

#include "PlotStructures.h"

#include <cstdint>

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui.h>

/*
 * Highly inspired by imgui_plot by Anton Lobashev.
 * https://github.com/soulthreads/imgui-plot
 *
 * This is a version where you plot (x,y) defined lines where the x coordinates is itself
 * driving the position of the point.
 */
namespace ImGui
{
    struct PlotSignalConfig
    {
        PlotValues<double> values;
        PlotScale scale;
        ImVec2 frame_size = ImVec2(0.f, 0.f);
        float line_thickness = 1.f;
    };

    IMGUI_API void PlotSignal(const PlotSignalConfig& config);
} // namespace ImGui

#endif //MICRALN_SIGNALPLOT_H
