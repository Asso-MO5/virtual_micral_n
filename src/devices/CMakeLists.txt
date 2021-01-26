cmake_minimum_required(VERSION 3.16)

project(micraln-devices
        DESCRIPTION "The devices for the Micral N emulation"
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

# Devices
set(DEVICES_TEST_NAME ${DEVICES_LIB_NAME}_tests)

set(DEVICES_FILES src/dummy.cpp)
set(DEVICES_TEST_FILES tests/dummy_test.cpp)

add_library(${DEVICES_LIB_NAME} ${DEVICES_FILES})

add_executable(${DEVICES_TEST_NAME} ${DEVICES_TEST_FILES})
set_target_properties(${DEVICES_TEST_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${DEVICES_TEST_NAME} gtest gtest_main ${EMULATION_LIB_NAME})
target_include_directories(${DEVICES_TEST_NAME} PRIVATE src/)

add_test(NAME ${DEVICES_TEST_NAME} COMMAND ${DEVICES_TEST_NAME})
