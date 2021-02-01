#ifndef MICRALN_SIGNALPLOT_H
#define MICRALN_SIGNALPLOT_H

#include <cstdint>
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
        struct Values
        {
            const float* x_series = nullptr;
            const float* y_series = nullptr;
            int count{};
            int offset = 0;
        } values;
        struct Scale
        {
            float x_min{};
            float x_max{};
            float y_min{};
            float y_max{};
            bool x_scaled = false;
        } scale;
        ImVec2 frame_size = ImVec2(0.f, 0.f);
        float line_thickness = 1.f;
    };

    IMGUI_API void PlotSignal(const PlotSignalConfig& config);
} // namespace ImGui

#endif //MICRALN_SIGNALPLOT_H
