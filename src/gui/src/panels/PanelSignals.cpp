#include "PanelSignals.h"

#include "gui/src/SignalPlot.h"
#include "gui/src/Simulator.h"

namespace
{
    double get_most_recent_time(const RecorderCollection& recorders)
    {
        double time = 0.0;

        for (const auto& recorder : recorders)
        {
            const size_t last_index = recorder.second->size() - 1;
            time = std::max(time, recorder.second->time_series()[last_index]);
        }
        return time;
    }

} // namespace

void display_signals_panel(Simulator& simulator)
{
    const auto& recorders = simulator.get_recorders();

    const auto most_recent_time = get_most_recent_time(recorders);
    const auto starting_time_for_frame =
            std::max(0.0, most_recent_time - recorders.get_time_frame_as_counter());

    ImGui::PlotSignalConfig config;
    config.scale.x_scaled = true;
    config.scale.x_min = starting_time_for_frame;
    config.scale.x_max = most_recent_time;
    config.scale.y_min = 0.f;
    config.scale.y_max = 1.f;
    config.frame_size = ImVec2(400, 25);
    config.line_thickness = 1.f;

    ImGui::Begin("Pluribus Signals");

    for (const auto& recorder : recorders)
    {
        config.values.count = recorder.second->size();
        config.values.x_series = recorder.second->time_series();
        config.values.y_series = recorder.second->state_series();
        ImGui::PlotSignal(config);

        ImGui::SameLine();
        ImGui::Text("%s", recorder.first.c_str());
    }

    ImGui::End();
}
