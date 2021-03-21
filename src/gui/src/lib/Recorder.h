#ifndef MICRALN_RECORDER_H
#define MICRALN_RECORDER_H

#include <cstdint>

class Recorder
{
public:
    [[nodiscard]] virtual std::size_t size() const = 0;
    [[nodiscard]] virtual const double* time_series() const = 0;
    [[nodiscard]] virtual const double* state_series() const = 0;

    void pause();
    void resume();

protected:
    [[nodiscard]] bool is_paused() const;

private:
    bool paused{};
};

#endif //MICRALN_RECORDER_H
