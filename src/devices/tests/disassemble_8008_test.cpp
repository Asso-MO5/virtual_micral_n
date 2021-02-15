#include "Disassemble8008.h"

#include "gmock/gmock.h"

using namespace testing;

TEST(Disassemble8008, needs_to_be_associated_to_a_data_view) {
    std::vector<std::uint8_t> data{0x00};
    Disassemble8008 disassemble{data};

    std::string instruction = disassemble.get(0x0000);

    ASSERT_THAT(instruction, Eq("HLT"));
}


TEST(Disassemble8008, decodes_immediate_8bit_values) {
    std::vector<std::uint8_t> data{0x2e, 0xf0};
    Disassemble8008 disassemble{data};

    std::string instruction = disassemble.get(0x0000);

    ASSERT_THAT(instruction, Eq("LHI $f0"));
}

TEST(Disassemble8008, decodes_immediate_16bit_values) {
    std::vector<std::uint8_t> data{0x44, 0x00, 0x20};
    Disassemble8008 disassemble{data};

    std::string instruction = disassemble.get(0x0000);

    ASSERT_THAT(instruction, Eq("JMP $2000"));
}

TEST(Disassemble8008, decodes_rst) {
    std::vector<std::uint8_t> data{0x25};
    Disassemble8008 disassemble{data};

    std::string instruction = disassemble.get(0x0000);

    ASSERT_THAT(instruction, Eq("RST $20"));
}

TEST(Disassemble8008, decodes_inp) {
    std::vector<std::uint8_t> data{0x43};
    Disassemble8008 disassemble{data};

    std::string instruction = disassemble.get(0x0000);

    ASSERT_THAT(instruction, Eq("INP $1"));
}

TEST(Disassemble8008, decodes_out) {
    std::vector<std::uint8_t> data{0x7f};
    Disassemble8008 disassemble{data};

    std::string instruction = disassemble.get(0x0000);

    ASSERT_THAT(instruction, Eq("OUT $17"));
}
