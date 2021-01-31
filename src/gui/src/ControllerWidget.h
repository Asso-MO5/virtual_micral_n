#ifndef MICRALN_CONTROLLERWIDGET_H
#define MICRALN_CONTROLLERWIDGET_H

class ControllerWidget
{
public:
    explicit ControllerWidget(bool& running);
    void update();

private:
    bool& running;
    bool step = false;
};

#endif //MICRALN_CONTROLLERWIDGET_H
