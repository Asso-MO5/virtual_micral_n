cmake_minimum_required(VERSION 3.13)

project(micraln-mcs8
        DESCRIPTION "The MCS-8 processor helpers for the Micral N emulation"
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

# MCS8
set(MCS8_TEST_NAME ${MCS8_LIB_NAME}_tests)

set(MCS8_FILES
        src/dummy.cpp)

set(MCS8_TEST_FILES
        tests/dummy.cpp
        )

add_library(${MCS8_LIB_NAME} ${MCS8_FILES})
target_include_directories(${MCS8_LIB_NAME} PRIVATE ../)
set_warnings(${MCS8_LIB_NAME})

add_executable(${MCS8_TEST_NAME} ${MCS8_TEST_FILES})
set_target_properties(${MCS8_TEST_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${MCS8_TEST_NAME} PRIVATE gtest gtest_main gmock PUBLIC ${MCS8_LIB_NAME})
target_include_directories(${MCS8_TEST_NAME} PRIVATE src/)
target_include_directories(${MCS8_TEST_NAME} PRIVATE ../)

add_test(NAME ${MCS8_TEST_NAME} COMMAND ${MCS8_TEST_NAME})
