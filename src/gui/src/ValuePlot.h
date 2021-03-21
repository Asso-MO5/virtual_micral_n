#ifndef MICRALN_VALUEPLOT_H
#define MICRALN_VALUEPLOT_H

#include <cstdint>
#include <imgui.h>

namespace ImGui
{
    struct PlotValueConfig
    {
        struct Values
        {
            const double* x_value_series = nullptr;
            const double* y_value_series = nullptr;
            const double* x_owner_series = nullptr;
            const uint32_t* y_owner_series = nullptr;

            int value_count{};
            int owner_count{};
            int offset = 0;
        } values;
        struct Scale
        {
            double x_min{};
            double x_max{};
            double y_min{};
            double y_max{};
            bool x_scaled = false;
        } scale;
        ImVec2 frame_size = ImVec2(0.f, 0.f);
        float line_thickness = 1.f;
    };

    IMGUI_API void PlotValue(const PlotValueConfig& config);
} // namespace ImGui

#endif //MICRALN_VALUEPLOT_H
