#include "Panel8008.h"

#include <cinttypes>
#include <devices/src/CPU8008.h>
#include <devices/src/ProcessorCard.h>
#include <emulator/src/Simulator.h>
#include <imgui.h>

const char* state_to_name(std::uint32_t state) { return STATE_NAMES[state]; }

void display_8008_panel(const Simulator& simulator, uint64_t average_frequency)
{
    const auto& scheduler = simulator.get_scheduler();
    const auto& clock_pulse = simulator.get_processor_card().get_debug_data().clock_pulse;
    const auto& cpu = simulator.get_processor_card().get_cpu();

    ImGui::Begin("8008");
    ImGui::Text("Time %" PRIu64 "ms", scheduler.get_counter().get() / 1000 / 1000);

    ImGui::Text("Clock frequency %" PRIu64 "kHz (real: %" PRIu64 "kHz)",
                scheduler.get_counter() > 0
                        ? 1'000'000 * clock_pulse.get() / scheduler.get_counter().get()
                        : 0,
                average_frequency);

    auto cpu_debug_data = cpu.get_debug_data();

    ImGui::Text("Data Pins: %02x ",
                simulator.get_processor_card().get_cpu().data_pins.get_value()); // Ouch...

    {
        auto window_width =
                ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
        ImGui::BeginChild("ChildLeft-Internal", ImVec2(window_width * 0.5f, 160), true);

        auto state = static_cast<std::uint32_t>(*cpu.output_pins.state);
        ImGui::Text("State %1d%1d%1d (%s)", (state >> 2) & 1, (state >> 1) & 1, (state >> 0) & 1,
                    state_to_name(state));

        auto cycle_control = static_cast<std::uint32_t>(cpu.get_debug_data().cycle_control);
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
        auto window_width =
                ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;

        ImGui::BeginChild("ChildLeft-Stack", ImVec2(window_width * 0.5f, 180), true);

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

    ImGui::End();
}
