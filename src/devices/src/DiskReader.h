#ifndef MICRALN_DISKREADER_H
#define MICRALN_DISKREADER_H

#include <cstdint>
#include <emulation_core/src/Scheduling.h>

class DiskReader
{
public:
    using track_type = uint8_t;
    using sector_type = uint8_t;
    using data_provider_type = std::function<uint8_t(track_type, sector_type, size_t index)>;

    struct Config
    {
        track_type track_count{};
        data_provider_type data_provider;
    };

    enum StepDirection
    {
        Previous,
        Next
    };

    explicit DiskReader(const Config& config);

    void step_track(StepDirection direction);
    [[nodiscard]] uint8_t current_track() const;

    void seek_sector(uint8_t sector_num);
    sector_type current_sector() const;

    uint8_t read_data();
private:
    Config configuration;

    track_type track{};
    sector_type sector{};
    size_t index_in_sector{};
};

#endif //MICRALN_DISKREADER_H
