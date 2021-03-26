#ifndef MICRALN_VALUEPLOT_H
#define MICRALN_VALUEPLOT_H

#include "PlotStructures.h"

#include <cstdint>
#include <imgui.h>

namespace ImGui
{
    struct PlotValueConfig
    {
        PlotValues<double> data_values;
        PlotValues<uint32_t> owner_values;
        PlotScale scale;

        ImVec2 frame_size = ImVec2(0.f, 0.f);
        float line_thickness = 1.f;
        uint8_t bus_width;
    };

    IMGUI_API void PlotValue(const PlotValueConfig& config);
} // namespace ImGui

#endif //MICRALN_VALUEPLOT_H
