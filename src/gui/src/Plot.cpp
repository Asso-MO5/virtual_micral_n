#include "Plot.h"

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui_internal.h>

namespace ImGui
{
    std::tuple<bool, ImVec2, ImVec2> render_plot_frame(const ImVec2& frame_size)
    {
        ImGuiWindow* window = GetCurrentWindow();

        const ImGuiStyle& style = GImGui->Style;

        const ImRect frame_bounding_box(window->DC.CursorPos, window->DC.CursorPos + frame_size);
        const ImRect inner_bounding_box(frame_bounding_box.Min + style.FramePadding,
                                        frame_bounding_box.Max - style.FramePadding);
        const ImRect total_bounding_box = frame_bounding_box;
        ItemSize(total_bounding_box, style.FramePadding.y);
        if (!ItemAdd(total_bounding_box, 0, &frame_bounding_box))
        {
            return {false, ImVec2{}, ImVec2{}};
        }

        RenderFrame(frame_bounding_box.Min, frame_bounding_box.Max, GetColorU32(ImGuiCol_FrameBg),
                    true, style.FrameRounding);
        return {true, inner_bounding_box.Min, inner_bounding_box.Max};

    }
}
