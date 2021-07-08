#include "VirtualDisk.h"

VirtualDisk::VirtualDisk(std::span<uint8_t> data, Layout layout)
    : data{begin(data), end(data)}, layout{layout}
{
    initialize_checksums();
}

std::uint8_t VirtualDisk::get(int track, int sector, int index_in_sector) const
{
    if (index_in_sector == 0)
    {
        return 0xff;
    }
    if (index_in_sector == 1)
    {
        return 0xfe;
    }
    if (index_in_sector == layout.sector_size + 1 + 2)
    {
        return get_checksum(track, sector);
    }

    const size_t global_index =
            layout.sector_size * ((track * layout.sectors) + sector) + (index_in_sector - 2);
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
            uint8_t sector_checksum = 0;

            for (auto index = 0; index < layout.sector_size; index += 1)
            {
                const size_t global_index =
                        layout.sector_size * ((track_count * layout.sectors) + sector_count) +
                        index;

                const auto read_byte = global_index < data.size() ? data[global_index] : 0;
                const auto crc = sector_checksum;
                const auto for_carry = ((read_byte ^ crc) ^ 0xff) & ((read_byte + crc) ^ read_byte);
                const std::uint8_t carry = (for_carry & 0x80) ? 1 : 0;
                sector_checksum = crc + read_byte + carry;
            }

            checksums[track_count * layout.sectors + sector_count] = sector_checksum;
        }
    }
}

std::uint8_t VirtualDisk::get_checksum(int track, int sector) const
{
    return checksums[track * layout.sectors + sector];
    ;
}
