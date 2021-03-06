#include "Panel8008.h"

#include "gui/src/SignalPlot.h"
#include "gui/src/Simulator.h"

#include <devices/src/CPU8008.h>
#include <devices/src/ProcessorCard.h>
#include <emulation_core/src/DataBus.h>
#include <imgui.h>

const char* state_to_name(uint state) { return STATE_NAMES[state]; }

void display_8008_panel(const Simulator& simulator, uint64_t average_frequency)
{
    static bool running_update = true;

    const auto& scheduler = simulator.get_scheduler();
    const auto& clock_1_pulse = simulator.clock_1_pulse;
    const auto& clock_2_pulse = simulator.clock_2_pulse;

    const auto& phase_1_recorder = simulator.phase_1_recorder;
    const auto& phase_2_recorder = simulator.phase_2_recorder;
    const auto& sync_recorder = simulator.sync_recorder;
    const auto& t3prime_recorder = simulator.t3prime_recorder;

    const auto& cpu = simulator.get_processor_card().get_cpu();

    ImGui::Begin("8008");
    ImGui::Text("Time %lu ms", scheduler.get_counter() / 1000 / 1000);

    ImGui::Text("Clock frequency %lu kHz (real: %lu kHz)",
                scheduler.get_counter() > 0 ? 1'000'000 * clock_1_pulse / scheduler.get_counter()
                                            : 0,
                average_frequency);

    ImGui::Checkbox("Update while running", &running_update);

    if (running_update)
    {
        const double first_time =
                std::min(phase_1_recorder.time_series()[0], phase_2_recorder.time_series()[0]);
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

        for (const auto& recorder :
             {phase_1_recorder, phase_2_recorder, sync_recorder, t3prime_recorder})
        {
            config.values.x_series = recorder.time_series();
            config.values.y_series = recorder.state_series();
            ImGui::PlotSignal(config);
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