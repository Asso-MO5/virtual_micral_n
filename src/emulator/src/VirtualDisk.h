#ifndef MICRALN_VIRTUALDISK_H
#define MICRALN_VIRTUALDISK_H

#include <cstdint>
#include <span>
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
    std::vector<uint8_t> data;
    std::vector<uint8_t> checksums;
    Layout layout;

    void initialize_checksums();
    [[nodiscard]] std::uint8_t get_checksum(int track, int sector) const;
    uint8_t computer_sector_checksum(int track, int sector);

    [[nodiscard]] std::uint8_t get_data(int track, int sector, int index_in_sector) const;

};

#endif //MICRALN_VIRTUALDISK_H
