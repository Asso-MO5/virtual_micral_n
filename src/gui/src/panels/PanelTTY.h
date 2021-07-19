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
};

#endif //MICRALN_PANELTTY_H
