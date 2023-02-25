#include "PanelTTY.h"

#include <emulator/src/Simulator.h>
#include <emulator/src/VirtualTTY.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <misc_utils/src/ToHex.h>

namespace
{
    void acquire_keyboard(VirtualTTY& tty, bool echo, std::string& content)
    {
        ImGuiIO& io = ImGui::GetIO();
        for (int i = 0; i < io.InputQueueCharacters.Size; i++)
        {
            ImWchar c = io.InputQueueCharacters[i];
            tty.emit_char(static_cast<char>(c & 0xff));

            if (echo)
            {
                content.push_back(c & 0xff);
            }
        }

        if (ImGui::IsKeyPressedMap(ImGuiKey_Enter) || ImGui::IsKeyPressedMap(ImGuiKey_KeyPadEnter))
        {
            tty.emit_char(0x0d);
        }
    }

    std::string representation(char c, bool raw_output)
    {
        if (!raw_output)
        {
            c &= (c == -1) ? -1 : 0x7f;
        }

        if (c == 13)
        {
            return {"\n"};
        }
        if (c == 10)
        {
            return {'\n'};
        }

        if (c == 0x11)
        {
            return {"\n==(DC1)==\n"};
        }
        if (c == 0x12 && !raw_output)
        {
            return {"\n==(DC2)==\n"};
        }
        if (c == 0x14 && raw_output)
        {
            return {"\n==(DC4)==\n"};
        }

        if (raw_output)
        {
            return "¬";
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
    bool text_was_added = false;

    if (tty_content.size() > previous_content_size)
    {
        const auto new_content = tty_content.substr(previous_content_size);
        previous_content_size = tty_content.size();

        for (auto c : new_content)
        {
            if (ff_count > 0)
            {
                if (c == -1)
                {
                    ff_count += 1;
                }
                else
                {
                    if (ff_count > 10)
                    {
                        ff_count = 0;
                    }
                }
            }

            content.append(representation(c, ff_count > 0));

            if (c == 0x12 && !(ff_count > 0))
            {
                ff_count = 1;
            }
            else if (c == 0x14 && (ff_count > 0))
            {
                ff_count = 0;
            }
        }

        text_was_added = true;
    }

    ImGui::Begin("TTY");
    static bool echo = false;
    ImGui::Checkbox("Echo", &echo);

    ImGui::BeginChild("Content");
    ImGui::TextWrapped("%s", content.c_str());

    if (text_was_added)
    {
        ImGui::SetScrollHereY(1.f);
    }
    ImGui::EndChild();

    ImGui::End();

    acquire_keyboard(tty, echo, content);
}
