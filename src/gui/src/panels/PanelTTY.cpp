#include "PanelTTY.h"

#include <emulator/src/Simulator.h>
#include <emulator/src/VirtualTTY.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <misc_utils/src/ToHex.h>

namespace
{
    void acquire_keyboard(VirtualTTY& tty)
    {
        ImGuiIO& io = ImGui::GetIO();
        for (int i = 0; i < io.InputQueueCharacters.Size; i++)
        {
            ImWchar c = io.InputQueueCharacters[i];
            tty.emit_char(static_cast<char>(c & 0xff));
        }

        if (ImGui::IsKeyPressedMap(ImGuiKey_Enter) || ImGui::IsKeyPressedMap(ImGuiKey_KeyPadEnter))
        {
            tty.emit_char(0x0d);
        }
    }

    std::string representation(char c, bool punch_started)
    {
        if (c == 13)
        {
            return {"\n"};
        }
        if (c == 10)
        {
            return {};
        }

        if (c == 0x11)
        {
            return {"\n==(DC1)==\n"};
        }
        if (c == 0x12 && !punch_started)
        {
            return {"\n==(DC2)==\n"};
        }
        if (c == 0x14 && punch_started)
        {
            return {"\n==(DC4)==\n"};
        }

        if (c < 32)
        {
            auto value = utils::to_hex<std::uint8_t>(c, 2);

            std::stringstream s;
            s << '(' << value << ')';
            return s.str();
        }

        return {c};
    }
}

void PanelTTY::display(Simulator& simulator)
{
    auto tty = simulator.get_virtual_tty();

    const auto& tty_content = tty.content();

    if (tty_content.size() > previous_content_size)
    {
        const auto new_content = tty_content.substr(previous_content_size);
        previous_content_size = tty_content.size();

        for (auto c : new_content)
        {
            content.append(representation(c, punch_started));
            if (!punch_started)
            {
                punch_started |= (c == 0x12);
            }
            if (punch_started)
            {
                punch_started &= (c != 0x14);
            }
        }
    }

    ImGui::Begin("TTY");
    ImGui::TextWrapped("%s", content.c_str());
    ImGui::End();

    acquire_keyboard(tty);
}
