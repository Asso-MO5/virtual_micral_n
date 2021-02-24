#include "PanelControl.h"

#include "Simulator.h"
#include "widgets/PanelLed.h"
#include "widgets/PanelSwitch.h"

#include <devices/src/CPU8008.h>
#include <devices/src/IOController.h>
#include <imgui.h>
#include <numeric>

using namespace widgets;

void PanelControl::display(Simulator& simulator)
{
    ImGui::Begin("Panel");

    auto interrupt_value_before = interrupt_value;

    ImGui::BeginGroup();
    ImGui::Text("INT");
    display_control_button("INT", &interrupt_value, IMPULSE);
    ImGui::EndGroup();

    if (interrupt_value != interrupt_value_before)
    {
        auto time = simulator.get_scheduler().get_counter();
        Edge::Front front = interrupt_value ? Edge::Front::RISING : Edge::Front::FALLING;
        simulator.get_interrupt_controller().wants_interrupt(Edge{front, time});
    }

    ImGui::SameLine();

    auto wait_value_before = wait_value;

    ImGui::BeginGroup();
    ImGui::Text("WAIT");
    display_control_button("WAIT", &wait_value, TOGGLE);
    ImGui::EndGroup();

    if (wait_value_before != wait_value)
    {
        auto time = simulator.get_scheduler().get_counter();
        Edge::Front front = wait_value ? Edge::Front::FALLING : Edge::Front::RISING;
        simulator.set_wait_line(Edge{front, time});
    }

    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Text("INPUT");

    for (auto& input_button : inputs)
    {
        display_control_button("B", &input_button, TOGGLE);
        ImGui::SameLine();
    }
    ImGui::EndGroup();

    auto input_data = std::accumulate(begin(inputs), end(inputs), 0,
                                      [](auto a, auto b) { return a << 1 | b; });

    simulator.get_io_controller().set_data_to_send(input_data);

    auto& cpu = simulator.get_cpu();
    auto& output_pins = cpu.get_output_pins();

    ImGui::BeginGroup();
    ImGui::Text("STOPPED");
    display_led(output_pins.state == Constants8008::CpuState::STOPPED, GREEN);
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginGroup();
    ImGui::Text("WAIT");
    display_led(output_pins.state == Constants8008::CpuState::WAIT, GREEN);
    ImGui::EndGroup();

    ImGui::End();
}
