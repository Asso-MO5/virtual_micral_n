#ifndef MICRALN_RECORDER_H
#define MICRALN_RECORDER_H

#include <cstdint>
#include <cstdlib>

class Recorder
{
public:
    [[nodiscard]] virtual std::size_t size() const = 0;
    [[nodiscard]] virtual const double* time_series() const = 0;
    [[nodiscard]] virtual const double* data_series() const = 0;

    [[nodiscard]] virtual std::size_t owner_size() const = 0;
    [[nodiscard]] virtual const double* owner_time_series() const = 0;
    [[nodiscard]] virtual const std::uint32_t* owner_data_series() const = 0;

    [[nodiscard]] virtual uint8_t bus_width() const = 0;

    void pause();
    void resume();

protected:
    [[nodiscard]] bool is_paused() const;

private:
    bool paused{};
};

#endif //MICRALN_RECORDER_H
