#ifndef MICRALN_VIRTUALDISK_H
#define MICRALN_VIRTUALDISK_H

#include <span>
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

    [[nodiscard]] std::uint8_t get(int track, int sector, int index_in_sector) const;

private:
    std::vector<uint8_t > data;
    Layout layout;
};

#endif //MICRALN_VIRTUALDISK_H
