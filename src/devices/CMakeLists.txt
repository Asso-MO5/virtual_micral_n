cmake_minimum_required(VERSION 3.16)

project(micraln-devices
        DESCRIPTION "The devices for the Micral N emulation"
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

# Devices
set(DEVICES_TEST_NAME ${DEVICES_LIB_NAME}_tests)

set(DEVICES_FILES
        src/Clock.cpp src/Clock.h
        )
set(DEVICES_TEST_FILES tests/clock_test.cpp)

add_library(${DEVICES_LIB_NAME} ${DEVICES_FILES})
target_include_directories(${DEVICES_LIB_NAME} PRIVATE ../)

add_executable(${DEVICES_TEST_NAME} ${DEVICES_TEST_FILES})
set_target_properties(${DEVICES_TEST_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${DEVICES_TEST_NAME} PRIVATE gtest gtest_main gmock PUBLIC ${DEVICES_LIB_NAME} ${EMULATION_LIB_NAME})
target_include_directories(${DEVICES_TEST_NAME} PRIVATE src/)
target_include_directories(${DEVICES_TEST_NAME} PRIVATE ../)

add_test(NAME ${DEVICES_TEST_NAME} COMMAND ${DEVICES_TEST_NAME})
