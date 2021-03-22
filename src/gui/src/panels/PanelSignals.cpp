#include "PanelSignals.h"

#include "gui/src/SignalPlot.h"
#include "gui/src/ValuePlot.h"

#include <emulator/src/Simulator.h>

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

    ImGui::PlotSignalConfig config_for_signal;
    config_for_signal.scale.x_scaled = true;
    config_for_signal.scale.x_min = starting_time_for_frame;
    config_for_signal.scale.x_max = most_recent_time;
    config_for_signal.scale.y_min = 0.f;
    config_for_signal.scale.y_max = 1.f;
    config_for_signal.frame_size = ImVec2(400, 25);
    config_for_signal.line_thickness = 1.f;

    ImGui::PlotValueConfig config_for_value;
    config_for_value.scale.x_scaled = true;
    config_for_value.scale.x_min = starting_time_for_frame;
    config_for_value.scale.x_max = most_recent_time;
    config_for_value.scale.y_min = 0.f;
    config_for_value.scale.y_max = 1.f;
    config_for_value.frame_size = ImVec2(400, 25);
    config_for_value.line_thickness = 1.f;

    ImGui::Begin("Pluribus Signals");

    for (const auto& recorder : recorders)
    {
        if (recorder.second->owner_size() == 0)
        {
            config_for_signal.values.count = recorder.second->size();
            config_for_signal.values.x_series = recorder.second->time_series();
            config_for_signal.values.y_series = recorder.second->data_series();
            ImGui::PlotSignal(config_for_signal);
        }
        else
        {
            config_for_value.data_values.count = recorder.second->size();
            config_for_value.data_values.x_series = recorder.second->time_series();
            config_for_value.data_values.y_series = recorder.second->data_series();

            config_for_value.owner_values.count = recorder.second->owner_size();
            config_for_value.owner_values.x_series = recorder.second->owner_time_series();
            config_for_value.owner_values.y_series = recorder.second->owner_data_series();

            ImGui::PlotValue(config_for_value);
        }
        ImGui::SameLine();
        ImGui::Text("%s", recorder.first.c_str());
    }

    ImGui::End();
}
