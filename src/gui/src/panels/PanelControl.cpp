#include "PanelControl.h"

#include "gui/src/widgets/PanelLed.h"
#include "gui/src/widgets/PanelSwitch.h"

#include <devices/src/CPU8008.h>
#include <devices/src/ConsoleCard.h>
#include <devices/src/IOController.h>
#include <devices/src/InterruptController.h>
#include <devices/src/ProcessorCard.h>
#include <emulator/src/Simulator.h>
#include <imgui.h>
#include <numeric>

using namespace widgets;

namespace
{
    std::array<int, 16> value_to_display_bits(uint16_t value, size_t bus_width)
    {
        std::array<int, 16> all_bits{};
        for (int i = 0; i < 16; i += 1)
        {
            all_bits[i] = value & 1;
            value >>= 1;
        }

        std::reverse(all_bits.data(), all_bits.data() + bus_width);

        return all_bits;
    }

    template<typename bit_array>
    uint16_t bits_to_value(const bit_array& array)
    {
        return std::accumulate(begin(array), end(array), 0,
                               [](auto a, auto b) { return a << 1 | b; });
    }

    template<typename switch_array>
    void display_led_and_switches(std::array<int, 16> all_bits, switch_array& all_switches)
    {
        const auto bus_width = all_switches.size();
        auto count = 0;
        for (auto& value : all_switches)
        {
            auto gap = ((bus_width - count) % 4) == 0 && (count != 0);
            if (gap)
            {
                ImGui::Text(" ");
                ImGui::SameLine();
            }
            const bool bit_value = all_bits[count];

            ImGui::BeginGroup();
            display_led(bit_value, GREEN);
            display_control_button("B", &value, TOGGLE);
            ImGui::EndGroup();
            ImGui::SameLine();

            count += 1;
        }
    }

    bool add_impulse_switch(const char* text, bool* switch_value, const bool* led_value)
    {
        auto previous_value = *switch_value;

        ImGui::BeginGroup();
        ImGui::Text("%s", text);
        if (led_value)
        {
            display_led(*led_value, GREEN);
        }
        display_control_button("B", switch_value, IMPULSE);
        ImGui::EndGroup();

        bool impulse = (*switch_value != previous_value) && *switch_value;

        return impulse;
    }

} // namespace

void PanelControl::display(Simulator& simulator)
{
    ImGui::Begin("Panel");

    ImGui::SameLine();

    // TODO: ultimately will only read the ConsoleCard
    const auto& cpu = simulator.get_processor_card().get_cpu();
    const auto& output_pins = cpu.output_pins;
    const auto& scheduler = simulator.get_scheduler();
    const auto& io_controller = simulator.get_io_controller();

    auto& console_card = simulator.get_console_card();

    {
        display_address_line(console_card);
        ImGui::NewLine();

        display_data_line(console_card);
        ImGui::NewLine();

        display_control_line(console_card);
        ImGui::NewLine();

        ImGui::BeginGroup();
        display_status_line(console_card, output_pins);
        display_av_init_line(simulator);

        ImGui::EndGroup();
    }

    ImGui::End();
}

void PanelControl::display_address_line(ConsoleCard& console_card)
{
    ImGui::BeginGroup();
    ImGui::Text("ADRESSE");

    const auto& status = console_card.get_status();
    uint16_t display_value = status.address;
    std::array<int, 16> all_bits{value_to_display_bits(display_value, input_address.size())};
    display_led_and_switches(all_bits, input_address);
    ImGui::EndGroup();

    auto numeric_data = bits_to_value(input_address);

    ImGui::Text("LED    Value: $%04X o%05o %5u", display_value, display_value, display_value);
    ImGui::Text("Switch Value: $%04X o%05o %5u", numeric_data, numeric_data, numeric_data);
}

void PanelControl::display_data_line(ConsoleCard& console_card)
{
    ImGui::BeginGroup();
    ImGui::Text("DONNEE");

    const auto& status = console_card.get_status();

    assert(status.is_op_cycle || status.is_read_cycle); // TODO: Other cycle display
    uint16_t display_value = status.is_op_cycle ? status.instruction : status.data;
    std::array<int, 16> all_bits{value_to_display_bits(display_value, input_data.size())};
    display_led_and_switches(all_bits, input_data);
    ImGui::EndGroup();

    auto numeric_data = bits_to_value(input_data);

    ImGui::Text("LED    Value: $%02X o%03o %3u", display_value, display_value, display_value);
    ImGui::Text("Switch Value: $%02X o%03o %3u", numeric_data, numeric_data, numeric_data);
}

void PanelControl::display_control_line(ConsoleCard& console_card)
{
    static const char* CONTROL_NAMES[] = {"AUTO", "P/P", "INST", "CYCLE", "PIEGE", "SUB"};

    const auto& status = console_card.get_status();

    const bool control_values[] = {
            status.automatic,
            status.stepping,
            status.step_mode == ConsoleCard::Instruction,
            status.step_mode == ConsoleCard::Cycle,
            false,
            false,
    };

    const std::function<void(ConsoleCard*)> button_actions[] = {
            &ConsoleCard::press_automatic,
            &ConsoleCard::press_stepping,
            &ConsoleCard::press_instruction,
            &ConsoleCard::press_cycle,
            nullptr,
            nullptr,
    };

    assert(IM_ARRAYSIZE(CONTROL_NAMES) == IM_ARRAYSIZE(control_values));

    ImGui::BeginGroup();
    for (auto i = 0; i < IM_ARRAYSIZE(CONTROL_NAMES); i += 1)
    {
        auto pressed =
                add_impulse_switch(CONTROL_NAMES[i], &control_switches[i], &control_values[i]);

        if (pressed)
        {
            auto& action = button_actions[i];
            if (action != nullptr)
            {
                action(&console_card);
            }
        }
        ImGui::SameLine();
    }
    ImGui::EndGroup();
}

void PanelControl::display_status_line(ConsoleCard& console_card,
                                       const CPU8008::OutputPins& output_pins)
{
    static const char* INFORMATION_NAMES[] = {"EXEC", "PAUSE", "ARRET", "OP", "LEC", "E/S", "ECR"};

    const auto& status = console_card.get_status();

    // TODO: change output_pins for ConsoleCard information
    const bool info_values[] = {false,
                                *output_pins.state == Constants8008::CpuState::WAIT,
                                *output_pins.state == Constants8008::CpuState::STOPPED,
                                status.is_op_cycle,
                                status.is_read_cycle,
                                status.is_io_cycle,
                                status.is_write_cycle};

    assert(IM_ARRAYSIZE(INFORMATION_NAMES) == IM_ARRAYSIZE(info_values));

    for (auto i = 0; i < IM_ARRAYSIZE(INFORMATION_NAMES); i += 1)
    {
        ImGui::BeginGroup();
        ImGui::Text("%s", INFORMATION_NAMES[i]);
        display_led(info_values[i], GREEN);

        ImGui::EndGroup();
        ImGui::SameLine();
    }
}

void PanelControl::display_av_init_line(Simulator& simulator)
{
    const auto& scheduler = simulator.get_scheduler();

    bool av = false;
    auto av_pressed = add_impulse_switch("AV", &av, nullptr);
    ImGui::SameLine();

    bool interrupt_value = false;
    auto init_pressed = add_impulse_switch("INIT", &interrupt_value, nullptr);
    if (init_pressed && interrupt_value)
    {
        auto time = scheduler.get_counter();
        Edge::Front front = interrupt_value ? Edge::Front::RISING : Edge::Front::FALLING;
        simulator.get_processor_card().get_interrupt_controller().wants_interrupt(
                Edge{front, time});
    }
}
