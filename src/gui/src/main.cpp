#include "ControllerWidget.h"
#include "ImGuiSDLGLContext.h"
#include "SignalPlot.h"
#include "Simulator.h"

#include <devices/src/CPU8008.h>

#include <array>
#include <imgui.h>

static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 720;

const char* STATE_NAMES[] = {
        "WAIT", "T3", "T1", "STOPPED", "T2", "T5", "T1I", "T4",
};

const char* state_to_name(uint state) { return STATE_NAMES[state]; }

void ImGuiBaseWindows(float average_frame_time, float* clear_color, bool show_demo_window)
{
    if (show_demo_window)
    {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        ImGui::Begin("Hello, world!");
        ImGui::Checkbox("Demo Window", &show_demo_window);
        ImGui::ColorEdit3("clear color", clear_color);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", average_frame_time,
                    ImGui::GetIO().Framerate);
        ImGui::End();
    }
}

int main(int, char**)
{
    auto context = ImGui_SDL_GL_Context{{WINDOW_WIDTH, WINDOW_HEIGHT}};

    bool show_demo_window = true;

    Simulator simulator;
    ControllerWidget controller;

    bool done = false;
    while (!done)
    {
        auto average_frame_time = 1000.0f / ImGui::GetIO().Framerate;
        simulator.step(average_frame_time, controller.get_state());

        done = context.process_events([](const SDL_Event& event) {
            // Handle custom events.
        });

        context.start_imgui_frame();

        ImGuiBaseWindows(average_frame_time, static_cast<float*>(&context.clear_color.x),
                         show_demo_window);

        {
            const auto& scheduler = simulator.get_scheduler();
            const auto& clock_1_pulse = simulator.clock_1_pulse;
            const auto& clock_2_pulse = simulator.clock_2_pulse;

            const auto& phase_1_recorder = simulator.phase_1_recorder;
            const auto& phase_2_recorder = simulator.phase_2_recorder;
            const auto& sync_recorder = simulator.sync_recorder;

            const auto& cpu = simulator.get_cpu();

            ImGui::Begin("Scheduler");
            ImGui::Text("Time %lu ms", scheduler.get_counter() / 1000);
            if (scheduler.get_counter() > 0)
            {
                ImGui::Text("Clock frequency %lu kHz",
                            1'000'000 * clock_1_pulse / scheduler.get_counter());
            }

            const float first_time =
                    std::min(phase_1_recorder.time_series()[0], phase_2_recorder.time_series()[0]);
            const size_t last_index = phase_1_recorder.size() - 1;
            const float last_time = std::max(phase_1_recorder.time_series()[last_index],
                                             phase_2_recorder.time_series()[last_index]);

            ImGui::PlotSignalConfig config;
            config.values.count = phase_1_recorder.size();
            config.scale.x_scaled = true;
            config.scale.x_min = first_time;
            config.scale.x_max = last_time;
            config.scale.y_min = 0.f;
            config.scale.y_max = 1.f;
            config.frame_size = ImVec2(400, 25);
            config.line_thickness = 1.f;

            for (const auto& recorder : {phase_1_recorder, phase_2_recorder, sync_recorder})
            {
                config.values.x_series = recorder.time_series();
                config.values.y_series = recorder.state_series();
                ImGui::PlotSignal(config);
            }

            ImGui::Text("Data Bus %02x ", cpu.get_data_pins().data);

            auto state = static_cast<uint>(cpu.get_output_pins().state);
            ImGui::Text("State %1d%1d%1d (%s)", (state >> 2) & 1, (state >> 1) & 1,
                        (state >> 0) & 1, state_to_name(state));

            ImGui::End();
        }

        controller.update();
        context.render_frame();
    }

    return 0;
}
