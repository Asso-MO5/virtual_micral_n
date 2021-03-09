#include "Panel8008.h"

#include "gui/src/SignalPlot.h"
#include "gui/src/Simulator.h"

#include <devices/src/CPU8008.h>
#include <devices/src/ProcessorCard.h>
#include <emulation_core/src/DataBus.h>
#include <imgui.h>

const char* state_to_name(uint state) { return STATE_NAMES[state]; }

double get_most_recent_time(const RecorderCollection& recorders)
{
    double time = 0.0;

    for (const auto& recorder : recorders)
    {
        const size_t last_index = recorder.second.size() - 1;
        time = std::max(time, recorder.second.time_series()[last_index]);
    }
    return time;
}

void display_8008_panel(const Simulator& simulator, uint64_t average_frequency)
{
    static bool running_update = true;

    const auto& scheduler = simulator.get_scheduler();
    const auto& clock_pulse = simulator.get_processor_card().get_debug_info().clock_pulse;
    const auto& cpu = simulator.get_processor_card().get_cpu();

    ImGui::Begin("8008");
    ImGui::Text("Time %lu ms", scheduler.get_counter() / 1000 / 1000);

    ImGui::Text("Clock frequency %lu kHz (real: %lu kHz)",
                scheduler.get_counter() > 0 ? 1'000'000 * clock_pulse / scheduler.get_counter() : 0,
                average_frequency);

    ImGui::Checkbox("Update while running", &running_update);

    if (running_update)
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

        for (const auto& recorder : recorders)
        {
            config.values.count = recorder.second.size();
            config.values.x_series = recorder.second.time_series();
            config.values.y_series = recorder.second.state_series();
            ImGui::PlotSignal(config);

            ImGui::SameLine();
            ImGui::Text("%s", recorder.first.c_str());
        }

        auto cpu_debug_data = cpu.get_debug_data();

        ImGui::Text("Data Bus: %02x ", simulator.get_data_bus().read());

        {
            ImGui::BeginChild("ChildLeft-Internal",
                              ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 160), true);

            auto state = static_cast<uint>(*cpu.get_output_pins().state);
            ImGui::Text("State %1d%1d%1d (%s)", (state >> 2) & 1, (state >> 1) & 1,
                        (state >> 0) & 1, state_to_name(state));

            auto cycle_control = static_cast<uint>(cpu.get_debug_data().cycle_control);
            ImGui::Text("Cycle Control: %s",
                        CYCLE_CONTROL_NAMES[static_cast<size_t>(cycle_control >> 6)]);

            ImGui::Text("REG.a: %02x", cpu_debug_data.hidden_registers.a);
            ImGui::Text("REG.b: %02x", cpu_debug_data.hidden_registers.b);

            for (auto flag_index = 0; flag_index < IM_ARRAYSIZE(FLAG_NAMES); flag_index += 1)
            {
                ImGui::Text("%s: %s", FLAG_NAMES[flag_index],
                            cpu_debug_data.flags[flag_index] ? "X" : "_");
            }

            ImGui::Text("%s @ %04x",
                        cpu_debug_data.instruction_register == 0
                                ? "---"
                                : instruction_to_string(cpu_debug_data.decoded_instruction).c_str(),
                        cpu_debug_data.latest_emitted_pci);

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
                              ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 180), true);

            ImGui::Text("Address Stack");
            size_t stack_index = 0;
            for (auto& address : cpu_debug_data.address_stack.stack)
            {
                ImGui::Text("%04x %s", cpu_debug_data.address_stack.stack[stack_index],
                            stack_index == cpu_debug_data.address_stack.stack_index ? "<-" : "");
                stack_index += 1;
            }

            ImGui::EndChild();
        }
    }

    ImGui::End();
}