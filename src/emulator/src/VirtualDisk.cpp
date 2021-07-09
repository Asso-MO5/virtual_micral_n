#include "VirtualDisk.h"
#include <cassert>

namespace
{
    size_t get_index(const VirtualDisk::Layout& layout, int track, int sector, int index_in_sector)
    {
        return layout.sector_size * ((track * layout.sectors) + sector) + index_in_sector;
    }
}

VirtualDisk::VirtualDisk(std::span<uint8_t> data, Layout layout)
    : data{begin(data), end(data)}, layout{layout}
{
    initialize_checksums();
}

std::uint8_t VirtualDisk::get(uint8_t track, uint8_t sector, uint8_t index_in_sector) const
{
    if (index_in_sector >= 2 && index_in_sector < layout.sector_size + 1 + 2)
    {
        return get_data(track, sector, index_in_sector - 2);
    }
    if (index_in_sector == layout.sector_size + 1 + 2)
    {
        return get_checksum(track, sector);
    }
    return 0xff - index_in_sector;
}

std::uint8_t VirtualDisk::get_data(uint8_t track, uint8_t sector, uint8_t index_in_sector) const
{
    assert(track < layout.tracks && "Track number too high");
    assert(sector < layout.sectors && "Sector number too high");
    assert(index_in_sector < (layout.sector_size + 3) && "Index in sector too high");

    const size_t global_index = get_index(layout, track, sector, index_in_sector);
    if (global_index < data.size())
    {
        return data[global_index];
    }
    return 0;
}

void VirtualDisk::initialize_checksums()
{
    checksums.resize(layout.tracks * layout.sectors);

    for (auto track_count = 0; track_count < layout.tracks; track_count += 1)
    {
        for (auto sector_count = 0; sector_count < layout.sectors; sector_count += 1)
        {
            const uint8_t sector_checksum = computer_sector_checksum(track_count, sector_count);
            checksums[track_count * layout.sectors + sector_count] = sector_checksum;
        }
    }
}

uint8_t VirtualDisk::computer_sector_checksum(int track, int sector)
{
    assert(track < layout.tracks && "Track number too high");
    assert(sector < layout.sectors && "Sector number too high");

    uint8_t sector_checksum = 0;

    for (auto index = 0; index < layout.sector_size; index += 1)
    {
        const auto read_byte = get_data(track, sector, index);
        const auto for_carry = ((read_byte ^ sector_checksum) ^ 0xff) & ((read_byte + sector_checksum) ^ read_byte);
        const uint8_t carry = (for_carry & 0x80) ? 1 : 0;
        sector_checksum = sector_checksum + read_byte + carry;
    }

    return sector_checksum;
}

std::uint8_t VirtualDisk::get_checksum(int track, int sector) const
{
    assert(track < layout.tracks && "Track number too high");
    assert(sector < layout.sectors && "Sector number too high");
    return checksums[track * layout.sectors + sector];
}
