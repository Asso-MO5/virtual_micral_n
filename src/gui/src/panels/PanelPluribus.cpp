#include "PanelPluribus.h"

#include "gui/src/Simulator.h"

#include <imgui.h>

const size_t PLURIBUS_SLOT_COUNT = 11;

const char* const card_names[]{"-",        "Processor Card",    "Memory Card",
                               "I/O Card", "Stack Memory Card", "Serial Card"};

const char* const slot_names[] = {"Slot 1", "Slot 2", "Slot 3", "Slot 4",  "Slot 5", "Slot 6",
                                  "Slot 7", "Slot 8", "Slot 9", "Slot 10", "Slot 11"};

void display_pluribus_panel(Simulator& simulator)
{
    static std::array<int, PLURIBUS_SLOT_COUNT> card_types{};

    ImGui::Begin("Cards");
    ImGui::Text("Cards on the bus:");

    for (size_t slot_index = 0; slot_index < IM_ARRAYSIZE(slot_names); slot_index += 1)
    {
        int* pointer_to_selected = (card_types.data() + slot_index);
        ImGui::Combo(slot_names[slot_index], pointer_to_selected, card_names,
                     IM_ARRAYSIZE(card_names));
    }

    ImGui::End();
}
