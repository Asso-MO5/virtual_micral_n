#ifndef MICRALN_PANELMEMORYDISPLAY_H
#define MICRALN_PANELMEMORYDISPLAY_H

#include <array>
#include <cstdint>
#include <functional>
#include <imgui.h>

struct MemoryDisplayParameters
{
    uint16_t start_address;
    uint16_t highlighted_memory;
    size_t memory_size;
    std::function<uint8_t(uint16_t)> memory_access_function;
};

template<uint16_t visual_width>
void memory_display(const char* widget_name, const MemoryDisplayParameters& parameters)
{
    uint16_t address = parameters.start_address;
    const uint16_t end_address = address + parameters.memory_size;

    std::array<char, visual_width + 1> char_view{};
    char_view.back() = 0;
    uint16_t memory_line = 0;

    ImGui::Begin(widget_name);

    while (address < end_address)
    {
        const uint8_t value = parameters.memory_access_function(address);

        if (address % visual_width == 0)
        {
            ImGui::Text("$%004x: ", address);
            memory_line = address;
        }

        auto char_equivalent = (value < 32 || value > 127) ? '.' : value;
        char_view[address - memory_line] = char_equivalent;

        ImGui::SameLine();

        if (address == parameters.highlighted_memory)
        {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%002x ", value);
        }
        else
        {
            ImGui::Text("%002x ", value);
        }

        address += 1;

        if (address % visual_width == 0)
        {
            ImGui::SameLine();
            ImGui::Text("  %s", char_view.data());
        }
    }

    ImGui::End();
}

#endif //MICRALN_PANELMEMORYDISPLAY_H
