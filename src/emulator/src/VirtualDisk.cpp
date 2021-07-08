#include "VirtualDisk.h"

VirtualDisk::VirtualDisk(std::span<uint8_t> data, Layout layout)
    : data{begin(data), end(data)}, layout{layout}
{}

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

    const size_t global_index =
            layout.sector_size * ((track * layout.sectors) + sector) + (index_in_sector - 2);
    if (global_index < data.size())
    {
        return data[global_index];
    }
    return 0;
}
