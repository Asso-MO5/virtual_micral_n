#include "DiskReader.h"

#include "gmock/gmock.h"

using namespace testing;

namespace
{
    uint8_t null_data_provider(DiskReader::track_type, DiskReader::sector_type, size_t)
    {
        return 0;
    }

    uint8_t test_data_provider(DiskReader::track_type track, DiskReader::sector_type sector, size_t index)
    {
        return 0xff - track - sector - index;
    }
}

TEST(DiskReader, starts_with_track_at_00)
{
    DiskReader::Config config{.track_count = 10, .data_provider = null_data_provider};
    DiskReader reader{config};

    ASSERT_THAT(reader.current_track(), Eq(0));
}

TEST(DiskReader, can_step_to_next_track)
{
    DiskReader::Config config{.track_count = 10, .data_provider = null_data_provider};
    DiskReader reader{config};

    reader.step_track(DiskReader::Next);

    ASSERT_THAT(reader.current_track(), Eq(1));
}

TEST(DiskReader, has_a_number_of_tracks)
{
    DiskReader::Config config{.track_count = 10, .data_provider = null_data_provider};
    DiskReader reader{config};

    reader.step_track(DiskReader::Previous);
    ASSERT_THAT(reader.current_track(), Eq(9));
}

TEST(DiskReader, reads_data)
{
    DiskReader::Config config{.track_count = 10, .data_provider = test_data_provider};
    DiskReader reader{config};

    ASSERT_THAT(reader.read_data(), Eq(0xff));
    ASSERT_THAT(reader.read_data(), Eq(0xfe));
    ASSERT_THAT(reader.read_data(), Eq(0xfd));
}

TEST(DiskReader, can_seek_sector)
{
    DiskReader::Config config{.track_count = 10, .data_provider = test_data_provider};
    DiskReader reader{config};

    reader.read_data();
    reader.read_data();
    reader.seek_sector(0x11);

    ASSERT_THAT(reader.read_data(), Eq(0xff - 0x11));
    ASSERT_THAT(reader.read_data(), Eq(0xff - 0x11 - 1));
    ASSERT_THAT(reader.current_sector(), Eq(0x11));
}
