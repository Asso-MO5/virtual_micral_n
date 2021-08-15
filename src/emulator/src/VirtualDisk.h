#ifndef MICRALN_VIRTUALDISK_H
#define MICRALN_VIRTUALDISK_H

#include "misc_utils/src/span_compat.h"
#include <cstdint>
#include <vector>

class VirtualDisk
{
public:
    struct Layout
    {
        std::uint8_t tracks;
        std::uint8_t sectors;
        std::uint8_t sector_size;
    };

    VirtualDisk(std::span<std::uint8_t> data, Layout layout);

    [[nodiscard]] std::uint8_t get(uint8_t track, uint8_t sector, uint8_t index_in_sector) const;

private:
    std::vector<uint8_t> data;
    std::vector<uint8_t> checksums;
    Layout layout;

    void initialize_checksums();
    [[nodiscard]] std::uint8_t get_checksum(int track, int sector) const;
    uint8_t computer_sector_checksum(int track, int sector);

    [[nodiscard]] std::uint8_t get_data(uint8_t track, uint8_t sector,
                                        uint8_t index_in_sector) const;
};

#endif //MICRALN_VIRTUALDISK_H
