#include "DiskReader.h"

DiskReader::DiskReader(const Config& config) : configuration{config} {}

void DiskReader::step_track(StepDirection direction)
{
    if (direction == Next)
    {
        track = (track + 1) % configuration.track_count;
    }
    else
    {
        track = ((track == 0) ? configuration.track_count : track) - 1;
    }
}

void DiskReader::seek_sector(uint8_t sector_num)
{
    if (sector_num != sector)
    {
        sector = sector_num;
        index_in_sector = 0;
    }
}

uint8_t DiskReader::read_data()
{
    auto data = configuration.data_provider(track, sector, index_in_sector);
    index_in_sector += 1;
    return data;
}

uint8_t DiskReader::current_track() const { return track; }

DiskReader::sector_type DiskReader::current_sector() const { return sector; }
