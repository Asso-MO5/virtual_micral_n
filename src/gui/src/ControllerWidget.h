#ifndef MICRALN_CONTROLLERWIDGET_H
#define MICRALN_CONTROLLERWIDGET_H

#include <emulator/src/SimulationRunType.h>

class ControllerWidget
{
public:
    void update();

    [[nodiscard]] SimulationRunType get_state() const;

private:
    SimulationRunType state{PAUSED};
};

#endif //MICRALN_CONTROLLERWIDGET_H
