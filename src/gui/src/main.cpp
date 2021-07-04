#include "ControllerWidget.h"

#include "ImGuiSDLGLContext.h"
#include "gui/src/lib/Averager.h"
#include "gui/src/panels/Panel8008.h"
#include "gui/src/panels/PanelControl.h"
#include "gui/src/panels/PanelDisassembly.h"
#include "gui/src/panels/PanelMemory.h"
#include "gui/src/panels/PanelPluribus.h"
#include "gui/src/panels/PanelProcessorCard.h"
#include "gui/src/panels/PanelSignals.h"
#include "gui/src/panels/PanelStackChannelCard.h"
#include "gui/src/panels/PanelDiskController.h"

#include <devices/src/ProcessorCard.h>
#include <emulator/src/Simulator.h>
#include <i8008/src/Disassemble8008.h>

#include <imgui.h>

static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 720;

uint64_t get_average_frequency(const Simulator& simulator,
                               Averager<uint64_t, 8>& frequency_averager,
                               float average_frame_time_in_ms)
{
    uint64_t average_frequency = 0;

    const auto& scheduler = simulator.get_scheduler();

    if (scheduler.get_counter() > 0)
    {
        static uint64_t previous_pulse_count = 0;
        const auto& clock_1_pulse = simulator.get_processor_card().get_debug_data().clock_pulse;

        uint64_t immediate_frequency = 0;
        if (average_frame_time_in_ms > 0)
        {
            immediate_frequency = 1'000'000 * (clock_1_pulse - previous_pulse_count) /
                                  static_cast<uint64_t>(average_frame_time_in_ms * 1000.f * 1000.f);
        }

        frequency_averager.push(immediate_frequency);
        average_frequency = frequency_averager.average();

        previous_pulse_count = clock_1_pulse;
    }

    return average_frequency;
}

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
    bool toggle_display_8008_panel = true;
    bool toggle_disassembly_panel = true;

    Simulator simulator{MICRAL_MIC_1};
    ControllerWidget controller;
    Disassemble8008 disassemble{simulator.get_memory_view()};
    PanelControl panel_control;

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

        done = context.process_events([](const SDL_Event&) {
            // Handle custom events.
        });

        context.start_imgui_frame();

        ImGuiBaseWindows(average_frame_time_in_ms, show_demo_window);

        {
            ImGui::Begin("Panels");
            ImGui::Checkbox("8008", &toggle_display_8008_panel);
            ImGui::Checkbox("Disassembly", &toggle_disassembly_panel);
            ImGui::End();
        }

        uint64_t average_frequency =
                get_average_frequency(simulator, frequency_averager, average_frame_time_in_ms);

        if (toggle_display_8008_panel)
        {
            display_8008_panel(simulator, average_frequency);
        }

        if (toggle_disassembly_panel)
        {
            display_disassembly_panel(simulator, disassemble);
        }

        display_pluribus_panel(simulator);
        display_memory_panel(simulator);
        panel_control.display(simulator);
        display_signals_panel(simulator);
        display_processor_card_panel(simulator);
        display_stack_channel_card_panel(simulator, 0);
        display_stack_channel_card_panel(simulator, 1);
        display_disk_controller(simulator.get_disk_controller_card());

        controller.update();
        context.render_frame();
    }

    return 0;
}
