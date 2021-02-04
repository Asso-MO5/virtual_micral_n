#ifndef MICRALN_CONTROLLERWIDGET_H
#define MICRALN_CONTROLLERWIDGET_H

class ControllerWidget
{
public:
    enum State {
        RUNNING,
        PAUSED,
        STEP_ONE_FRAME,
        STEP_ONE_CLOCK,
        STEP_ONE_STATE,
    };

    void update();

    [[nodiscard]] State get_state() const;

private:
    State state{RUNNING};
};

#endif //MICRALN_CONTROLLERWIDGET_H
