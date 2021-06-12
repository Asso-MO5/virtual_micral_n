cmake_minimum_required(VERSION 3.13)

project(micraln-file-utils
        DESCRIPTION "Utilities to handle files"
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

set(MISC_UTILS_TEST_NAME ${MISC_UTILS_LIB_NAME}_tests)

set(MISC_UTILS_FILES
        src/ToHex.h
        src/dummy.cpp)

set(MISC_UTILS_TEST_FILES
        tests/to_hex_test.cpp)

add_library(${MISC_UTILS_LIB_NAME} ${MISC_UTILS_FILES})
target_include_directories(${MISC_UTILS_LIB_NAME} PRIVATE ../)
set_warnings(${MISC_UTILS_LIB_NAME})

add_executable(${MISC_UTILS_TEST_NAME} ${MISC_UTILS_TEST_FILES})
set_target_properties(${MISC_UTILS_TEST_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${MISC_UTILS_TEST_NAME} PRIVATE gtest gtest_main gmock)
target_include_directories(${MISC_UTILS_TEST_NAME} PRIVATE src/)
target_include_directories(${MISC_UTILS_TEST_NAME} PRIVATE ../)

add_test(NAME ${MISC_UTILS_TEST_NAME} COMMAND ${MISC_UTILS_TEST_NAME})
