#include "ControllerWidget.h"
#include "ImGuiSDLGLContext.h"
#include "SignalPlot.h"
#include "Simulator.h"

#include <devices/src/CPU8008.h>

#include <array>
#include <gui/src/lib/Averager.h>
#include <imgui.h>

static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 720;

const char* STATE_NAMES[] = {"WAIT", "T3", "T1", "STOPPED", "T2", "T5", "T1I", "T4"};

const char* REGISTER_NAMES[] = {"A", "B", "C", "D", "E", "H", "L"};

const char* state_to_name(uint state) { return STATE_NAMES[state]; }

void ImGuiBaseWindows(float average_frame_time, bool& show_demo_window)
{
    if (show_demo_window)
    {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    {
        ImGui::Begin("Information");
        ImGui::Checkbox("Demo Window", &show_demo_window);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", average_frame_time,
                    ImGui::GetIO().Framerate);
        ImGui::End();
    }
}

int main(int, char**)
{
    auto context = ImGui_SDL_GL_Context{{WINDOW_WIDTH, WINDOW_HEIGHT}};

    bool show_demo_window = false;
    bool display_8008_panel = true;
    bool running_update = true;

    Simulator simulator;
    ControllerWidget controller;

    Averager<uint64_t, 8> frequency_averager{};

    bool done = false;
    while (!done)
    {
        auto average_frame_time_in_ms = 1000.0f / ImGui::GetIO().Framerate;
        /* Another method is to get the immediate Delta Time value
        ImGuiIO& io = ImGui::GetIO();
        auto average_frame_time = io.DeltaTime * 1000.f;
         */

        simulator.step(average_frame_time_in_ms, controller.get_state());

        done = context.process_events([](const SDL_Event& event) {
            // Handle custom events.
        });

        context.start_imgui_frame();

        ImGuiBaseWindows(average_frame_time_in_ms, show_demo_window);

        {
            ImGui::Begin("Panels");
            ImGui::Checkbox("8008", &display_8008_panel);
            ImGui::Checkbox("Update while running", &running_update);
            ImGui::End();
        }

        if (display_8008_panel)
        {
            const auto& scheduler = simulator.get_scheduler();
            const auto& clock_1_pulse = simulator.clock_1_pulse;
            const auto& clock_2_pulse = simulator.clock_2_pulse;

            const auto& phase_1_recorder = simulator.phase_1_recorder;
            const auto& phase_2_recorder = simulator.phase_2_recorder;
            const auto& sync_recorder = simulator.sync_recorder;

            const auto& cpu = simulator.get_cpu();

            ImGui::Begin("8008");
            ImGui::Text("Time %lu ms", scheduler.get_counter() / 1000 / 1000);
            if (scheduler.get_counter() > 0)
            {
                static uint64_t previous_pulse_count = 0;

                uint64_t immediate_frequency = 0;
                if (average_frame_time_in_ms > 0)
                {
                    immediate_frequency =
                            1'000'000 * (clock_1_pulse - previous_pulse_count) /
                            static_cast<uint64_t>(average_frame_time_in_ms * 1000.f * 1000.f);
                }

                frequency_averager.push(immediate_frequency);
                ImGui::Text("Clock frequency %lu kHz (real: %lu kHz)",
                            1'000'000 * clock_1_pulse / scheduler.get_counter(),
                            frequency_averager.average());

                previous_pulse_count = clock_1_pulse;
            }

            if (running_update)
            {
                const double first_time = std::min(phase_1_recorder.time_series()[0],
                                                  phase_2_recorder.time_series()[0]);
                const size_t last_index = phase_1_recorder.size() - 1;
                const double last_time = std::max(phase_1_recorder.time_series()[last_index],
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

                auto cpu_debug_data = cpu.get_debug_data();

                ImGui::Text("Data Bus: %02x ", simulator.get_data_bus().read());

                {
                    ImGui::BeginChild("ChildLeft-Internal",
                                      ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 160),
                                      true);

                    auto state = static_cast<uint>(cpu.get_output_pins().state);
                    ImGui::Text("State %1d%1d%1d (%s)", (state >> 2) & 1, (state >> 1) & 1,
                                (state >> 0) & 1, state_to_name(state));

                    ImGui::Text("REG.a: %02x", cpu_debug_data.hidden_registers.a);
                    ImGui::Text("REG.b: %02x", cpu_debug_data.hidden_registers.b);

                    static const char* flag_names[] = {"Carry ", "Zero  ", "Sign  ", "Parity"};
                    for (auto flag_index = 0; flag_index < IM_ARRAYSIZE(flag_names);
                         flag_index += 1)
                    {
                        ImGui::Text("%s: %s", flag_names[flag_index],
                                    cpu_debug_data.flags[flag_index] ? "X" : "_");
                    }
                    ImGui::EndChild();
                }

                ImGui::SameLine();
                {
                    ImGui::BeginChild("ChildRight", ImVec2(0.f, 160), true);

                    ImGui::Text("Registers");

                    for (uint8_t r = 0; r < CPU8008::SCRATCH_PAD_SIZE; r++)
                    {
                        ImGui::Text("%s: %02x", REGISTER_NAMES[r], cpu_debug_data.registers[r]);
                    }
                    ImGui::EndChild();
                }

                {
                    ImGui::BeginChild("ChildLeft-Stack",
                                      ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 180),
                                      true);

                    ImGui::Text("Address Stack");
                    size_t stack_index = 0;
                    for (auto& address : cpu_debug_data.address_stack.stack)
                    {
                        ImGui::Text("%04x %s", cpu_debug_data.address_stack.stack[stack_index],
                                    stack_index == cpu_debug_data.address_stack.stack_index ? "<-"
                                                                                            : "");
                        stack_index += 1;
                    }

                    ImGui::EndChild();
                }
            }

            ImGui::End();
        }

        controller.update();
        context.render_frame();
    }

    return 0;
}
