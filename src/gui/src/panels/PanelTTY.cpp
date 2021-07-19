#include "PanelTTY.h"

#include <emulator/src/Simulator.h>
#include <emulator/src/VirtualTTY.h>
#include <imgui.h>
#include <imgui_internal.h>

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
}

void PanelTTY::display(Simulator& simulator)
{
    auto tty = simulator.get_virtual_tty();

    // TODO: copy each time to optimize.
    content = tty.content();

    ImGui::Begin("TTY");
    ImGui::TextWrapped("%s", content.c_str());
    ImGui::End();

    acquire_keyboard(tty);
}
