#include "VirtualDisk.h"

#include <gmock/gmock.h>

using namespace testing;

namespace
{
    uint8_t test_data[] = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', // 8 bytes
            'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', // 8 bytes
            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', // 8 bytes
    };

    void fill_big_data(std::vector<uint8_t>& data)
    {
        uint8_t counter = 0;
        std::generate_n(std::back_inserter(data), 257, [&counter]() {
            counter = (counter + 1) % 26;
            return 'A' + counter;
        });
    }
}

TEST(VirtualDisk, is_created_with_data_and_layout)
{
    VirtualDisk::Layout layout{.tracks = 10, .sectors = 32, .sector_size = 128};
    VirtualDisk disk{test_data, layout};

    ASSERT_THAT(disk.get(0x9, 0x00, 2), Eq(0));
}

TEST(VirtualDisk, sector_starts_with_a_predefined_word)
{
    // TODO: The start word in chosen completely randomly at the moment.
    VirtualDisk::Layout layout{.tracks = 10, .sectors = 32, .sector_size = 128};
    VirtualDisk disk{test_data, layout};

    ASSERT_THAT(disk.get(0x0, 0x00, 0), Eq(0xff));
    ASSERT_THAT(disk.get(0x0, 0x00, 1), Eq(0xfe));
}

TEST(VirtualDisk, sector_continues_with_payload)
{
    VirtualDisk::Layout layout{.tracks = 10, .sectors = 32, .sector_size = 128};
    VirtualDisk disk{test_data, layout};

    for (auto index = 0; index < sizeof(test_data); index += 1)
    {
        ASSERT_THAT(disk.get(0x0, 0x00, index + 2), Eq(test_data[index]));
    }
}

TEST(VirtualDisk, big_data_continues_on_next_sector)
{
    std::vector<uint8_t> big_data;
    fill_big_data(big_data);

    VirtualDisk::Layout layout{.tracks = 10, .sectors = 32, .sector_size = 128};
    VirtualDisk disk{big_data, layout};

    for (auto index = 0; index < layout.sector_size; index += 1)
    {
        ASSERT_THAT(disk.get(0x0, 0x01, index + 2), Eq(big_data[index + 128]));
    }
    ASSERT_THAT(disk.get(0x0, 0x02, 2), Eq(big_data[256]));
}

TEST(VirtualDisk, big_data_continues_on_next_tracks)
{
    std::vector<uint8_t> big_data;
    fill_big_data(big_data);

    VirtualDisk::Layout layout{.tracks = 10, .sectors = 1, .sector_size = 128};
    VirtualDisk disk{big_data, layout};

    for (auto index = 0; index < layout.sector_size; index += 1)
    {
        ASSERT_THAT(disk.get(0x1, 0, index + 2), Eq(big_data[index + 128]));
    }
    ASSERT_THAT(disk.get(0x2, 0, 2), Eq(big_data[256]));
}

TEST(VirtualDisk, computes_checksum)
{
    std::vector<uint8_t> big_data;
    fill_big_data(big_data);

    VirtualDisk::Layout layout{.tracks = 10, .sectors = 32, .sector_size = 128};
    VirtualDisk disk{big_data, layout};

    ASSERT_THAT(disk.get(0x0, 0x00, 128 + 2 + 1), Eq(231));
    ASSERT_THAT(disk.get(0x0, 0x01, 128 + 2 + 1), Eq(209));
    ASSERT_THAT(disk.get(0x0, 0x02, 128 + 2 + 1), Eq(88));
    ASSERT_THAT(disk.get(0x0, 0x03, 128 + 2 + 1), Eq(0));
}
