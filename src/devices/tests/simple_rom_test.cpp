#include "SimpleROM.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(SimpleRom, receives_data)
{
    const std::vector<uint8_t> data = {0x70, 0x55};
    SimpleROM rom{data};

    ASSERT_THAT(rom.get_direct_data(0), Eq(data[0]));
    ASSERT_THAT(rom.get_direct_data(1), Eq(data[1]));
}

TEST(SimpleRom, doesnt_apply_data_at_start)
{
    const std::vector<uint8_t> data = {0x70, 0x55};
    SimpleROM rom{data};

    rom.set_address(0x0000);

    auto & data_pins = rom.get_data_pins();
    ASSERT_FALSE(data_pins.taken);
}

TEST(SimpleRom, contains_data_after_select_and_read)
{
    const std::vector<uint8_t> data = {0x70, 0x55};
    SimpleROM rom{data};

    rom.set_address(0x0000);

    rom.signal_chip_select(Edge{Edge::Front::RISING, 0});
    rom.signal_output_enable(Edge{Edge::Front::RISING, 10});

    // rom.step(); -> no need, the device is passive and immediate.

    auto & data_pins = rom.get_data_pins();
    ASSERT_THAT(data_pins.data, Eq(data[0]));
}

TEST(SimpleRom, doesnt_apply_data_when_not_selected)
{
    const std::vector<uint8_t> data = {0x70, 0x55};
    SimpleROM rom{data};

    rom.set_address(0x0000);
    rom.signal_chip_select(Edge{Edge::Front::RISING, 0});
    rom.signal_output_enable(Edge{Edge::Front::RISING, 10});

    // rom.step(); -> no need, the device is passive and immediate.

    rom.signal_chip_select(Edge{Edge::Front::FALLING, 100});

    auto & data_pins = rom.get_data_pins();
    ASSERT_FALSE(data_pins.taken);
}

TEST(SimpleRom, contains_other_data_when_changing_address)
{
    const std::vector<uint8_t> data = {0x70, 0x55};
    SimpleROM rom{data};

    rom.set_address(0x0000);
    rom.signal_chip_select(Edge{Edge::Front::RISING, 0});
    rom.signal_output_enable(Edge{Edge::Front::RISING, 10});

    // rom.step(); -> no need, the device is passive and immediate.

    rom.signal_chip_select(Edge{Edge::Front::FALLING, 100});

    rom.set_address(0x0001);
    rom.signal_chip_select(Edge{Edge::Front::RISING, 0});

    // rom.step(); -> no need, the device is passive and immediate.

    auto & data_pins = rom.get_data_pins();
    ASSERT_THAT(data_pins.data, Eq(data[1]));
}
