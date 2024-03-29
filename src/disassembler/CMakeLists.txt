cmake_minimum_required(VERSION 3.13)

project(micraln-disassembler
        DESCRIPTION "A disassembler for the 8008 with specific Micral N support"
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

set(DISASSEMBLER_EXE_NAME "disassembler")
set(DISASSEMBLER_TEST_EXE_NAME ${DISASSEMBLER_EXE_NAME}_tests)

set(DISASSEMBLER_FILES
        src/main.cpp
        src/OutputLine.cpp src/OutputLine.h)
set(DISASSEMBLER_TEST_FILES tests/main_test.cpp)

add_executable(${DISASSEMBLER_EXE_NAME} ${DISASSEMBLER_FILES})
set_target_properties(${DISASSEMBLER_EXE_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${DISASSEMBLER_EXE_NAME} ${I8008_LIB_NAME} ${FILE_UTILS_LIB_NAME} CLI11::CLI11)
target_include_directories(${DISASSEMBLER_EXE_NAME} PRIVATE ../)

if(LTO_result AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
    set_target_properties(${DISASSEMBLER_EXE_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    message(STATUS "IPO activated for ${DISASSEMBLER_EXE_NAME}")
endif()

set_warnings(${DISASSEMBLER_EXE_NAME})

install(TARGETS ${DISASSEMBLER_EXE_NAME}
        DESTINATION ${MICRAL_PACKAGE_DIR}
        CONFIGURATIONS Release
)

if(WITH_TESTS)
    add_executable(${DISASSEMBLER_TEST_EXE_NAME} ${DISASSEMBLER_TEST_FILES})
    target_link_libraries(${DISASSEMBLER_TEST_EXE_NAME} ${DEVICES_LIB_NAME} ${EMULATION_LIB_NAME} gtest gtest_main)
    if(${DISASSEMBLER_TEST_EXE_NAME})
        target_link_options(${DEVICES_TEST_NAME} PRIVATE -static-libgcc -static-libstdc++)
    endif()

    add_test(NAME ${DISASSEMBLER_TEST_EXE_NAME} COMMAND ${DISASSEMBLER_TEST_EXE_NAME})
endif()
