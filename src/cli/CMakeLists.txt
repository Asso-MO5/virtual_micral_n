cmake_minimum_required(VERSION 3.16)

project(micraln-cli
        DESCRIPTION "The command line interface for the Micral N emulation"
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

# CLI
set(CLI_EXE_NAME "micral_cli")
set(CLI_TEST_EXE_NAME ${CLI_EXE_NAME}_tests)

set(CLI_FILES src/main.cpp)
set(CLI_TEST_FILES tests/test_main.cpp)

add_executable(${CLI_EXE_NAME} ${CLI_FILES})
set_target_properties(${CLI_EXE_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${CLI_EXE_NAME} ${EMULATION_LIB_NAME} ${DEVICES_LIB_NAME} ${LOGURU_LIB_NAME})
target_include_directories(${CLI_EXE_NAME} PRIVATE ../)

if(LTO_result AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
    set_target_properties(${CLI_EXE_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    message("IPO activated for ${CLI_EXE_NAME}")
endif()

add_executable(${CLI_TEST_EXE_NAME} ${CLI_TEST_FILES})
target_link_libraries(${CLI_TEST_EXE_NAME} ${EMULATION_LIB_NAME} gtest gtest_main)


add_test(NAME ${CLI_TEST_EXE_NAME} COMMAND ${CLI_TEST_EXE_NAME})
