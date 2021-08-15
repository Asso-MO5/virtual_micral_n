cmake_minimum_required(VERSION 3.13)

project(micraln-file-utils
        DESCRIPTION "Utilities to handle files"
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

set(FILE_UTILS_TEST_NAME ${FILE_UTILS_LIB_NAME}_tests)

set(FILE_UTILS_FILES
        src/FileReader.cpp src/FileReader.h)

set(FILE_UTILS_TEST_FILES
        tests/file_reader_test.cpp)

add_library(${FILE_UTILS_LIB_NAME} ${FILE_UTILS_FILES})
target_include_directories(${FILE_UTILS_LIB_NAME} PRIVATE ../)
set_warnings(${FILE_UTILS_LIB_NAME})

add_executable(${FILE_UTILS_TEST_NAME} ${FILE_UTILS_TEST_FILES})
set_target_properties(${FILE_UTILS_TEST_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${FILE_UTILS_TEST_NAME}
        PRIVATE gtest gtest_main gmock)
target_include_directories(${FILE_UTILS_TEST_NAME} PRIVATE src/)
target_include_directories(${FILE_UTILS_TEST_NAME} PRIVATE ../)
if(${LINK_STATIC_EXE})
    target_link_options(${FILE_UTILS_TEST_NAME} PRIVATE -static-libgcc -static-libstdc++)
endif()

add_test(NAME ${FILE_UTILS_TEST_NAME} COMMAND ${FILE_UTILS_TEST_NAME})
