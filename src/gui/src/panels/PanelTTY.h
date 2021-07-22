#ifndef MICRALN_PANELTTY_H
#define MICRALN_PANELTTY_H

#include <string>

class Simulator;
class VirtualTTY;

class PanelTTY
{
public:
    void display(Simulator& simulator);

private:
    std::string content;
    size_t previous_content_size{};
    bool punch_started{};
};

#endif //MICRALN_PANELTTY_H
