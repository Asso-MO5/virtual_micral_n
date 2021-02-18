#include "SimpleRAM.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(SimpleRAM, has_a_debug_set_data)
{
    SimpleRAM rom{10};

    rom.set_direct_data(0x0000, 0x70);
    rom.set_direct_data(0x0001, 0x55);

    ASSERT_THAT(rom.get_direct_data(0), Eq(0x70));
    ASSERT_THAT(rom.get_direct_data(1), Eq(0x55));
}

TEST(SimpleRAM, doesnt_apply_data_at_start)
{
    SimpleRAM rom{10};

    rom.set_address(0x0000);

    auto & data_pins = rom.get_data_pins();
    ASSERT_FALSE(data_pins.is_owning_bus());
}

TEST(SimpleRAM, contains_data_after_select_and_read)
{
    SimpleRAM rom{10};
    rom.set_direct_data(0x0000, 0x70);

    rom.set_address(0x0000);

    rom.signal_chip_select(Edge{Edge::Front::RISING, 0});
    rom.signal_output_enable(Edge{Edge::Front::RISING, 10});

    // rom.step(); -> no need, the device is passive and immediate.

    auto & data_pins = rom.get_data_pins();
    ASSERT_TRUE(data_pins.is_owning_bus());
    ASSERT_THAT(data_pins.read(), Eq(0x70));
}

TEST(SimpleRAM, doesnt_apply_data_when_not_selected)
{
    SimpleRAM rom{10};

    rom.set_address(0x0000);
    rom.signal_chip_select(Edge{Edge::Front::RISING, 0});
    rom.signal_output_enable(Edge{Edge::Front::RISING, 10});

    // rom.step(); -> no need, the device is passive and immediate.

    rom.signal_chip_select(Edge{Edge::Front::FALLING, 100});

    auto & data_pins = rom.get_data_pins();
    ASSERT_FALSE(data_pins.is_owning_bus());
}

TEST(SimpleRAM, contains_other_data_when_changing_address)
{
    SimpleRAM rom{10};
    rom.set_direct_data(0x0000, 0x70);
    rom.set_direct_data(0x0001, 0x55);

    rom.set_address(0x0000);
    rom.signal_chip_select(Edge{Edge::Front::RISING, 0});
    rom.signal_output_enable(Edge{Edge::Front::RISING, 10});

    // rom.step(); -> no need, the device is passive and immediate.

    rom.signal_chip_select(Edge{Edge::Front::FALLING, 100});

    rom.set_address(0x0001);
    rom.signal_chip_select(Edge{Edge::Front::RISING, 0});

    // rom.step(); -> no need, the device is passive and immediate.

    auto & data_pins = rom.get_data_pins();
    ASSERT_TRUE(data_pins.is_owning_bus());
    ASSERT_THAT(data_pins.read(), Eq(0x55));
}

TEST(SimpleRAM, is_not_owning_the_bus_when_write_enabled)
{
    SimpleRAM rom{10};

    rom.set_address(0x0000);

    rom.signal_chip_select(Edge{Edge::Front::RISING, 0});
    rom.signal_write_enable(Edge{Edge::Front::RISING, 10});

    // rom.step(); -> no need, the device is passive and immediate.

    auto & data_pins = rom.get_data_pins();
    ASSERT_FALSE(data_pins.is_owning_bus());
}

TEST(SimpleRAM, write_enable_takes_precedence_on_output_enable)
{
    SimpleRAM rom{10};

    rom.set_address(0x0000);

    rom.signal_chip_select(Edge{Edge::Front::RISING, 0});
    rom.signal_output_enable(Edge{Edge::Front::RISING, 10});
    rom.signal_write_enable(Edge{Edge::Front::RISING, 20});

    // rom.step(); -> no need, the device is passive and immediate.

    auto & data_pins = rom.get_data_pins();
    ASSERT_FALSE(data_pins.is_owning_bus());
}


TEST(SimpleRAM, can_be_written_data_after_select_and_write_enable)
{
    SimpleRAM rom{10};
    //rom.set_direct_data(0x0000, 0x70);

    rom.set_address(0x0000);

    rom.signal_chip_select(Edge{Edge::Front::RISING, 0});
    rom.signal_write_enable(Edge{Edge::Front::RISING, 10});

    // rom.step(); -> no need, the device is passive and immediate.

    auto & data_pins = rom.get_data_pins();
    data_pins.write(0x70);

    rom.signal_write_enable(Edge{Edge::Front::FALLING, 30});

    ASSERT_FALSE(data_pins.is_owning_bus());
    ASSERT_THAT(rom.get_direct_data(0x0000), Eq(0x70));
}

TEST(SimpleRAM, can_be_written_at_different_adresses)
{
    SimpleRAM rom{10};


    auto & data_pins = rom.get_data_pins();

    rom.signal_chip_select(Edge{Edge::Front::RISING, 0});

    rom.set_address(0x0000);
    rom.signal_write_enable(Edge{Edge::Front::RISING, 10});
    // rom.step(); -> no need, the device is passive and immediate.
    data_pins.write(0x70);
    rom.signal_write_enable(Edge{Edge::Front::FALLING, 30});

    rom.set_address(0x0001);
    rom.signal_write_enable(Edge{Edge::Front::RISING, 50});
    // rom.step(); -> no need, the device is passive and immediate.
    data_pins.write(0x55);
    rom.signal_write_enable(Edge{Edge::Front::FALLING, 60});

    ASSERT_FALSE(data_pins.is_owning_bus());
    ASSERT_THAT(rom.get_direct_data(0x0000), Eq(0x70));
    ASSERT_THAT(rom.get_direct_data(0x0001), Eq(0x55));
}
