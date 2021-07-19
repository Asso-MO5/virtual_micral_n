#include "PanelTTY.h"

#include <emulator/src/Simulator.h>
#include <emulator/src/VirtualTTY.h>
#include <imgui.h>

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
