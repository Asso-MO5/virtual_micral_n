#ifndef MICRALN_PLOT_H
#define MICRALN_PLOT_H

#include <imgui.h>
#include <tuple>

namespace ImGui
{
    std::tuple<bool, ImVec2, ImVec2> render_plot_frame(const ImVec2& frame_size);
}

#endif //MICRALN_PLOT_H
