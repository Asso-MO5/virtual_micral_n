cmake_minimum_required(VERSION 3.13)

project(micraln-i8008
        DESCRIPTION "The MCS-8 processor helpers for the Micral N emulation"
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

# I8008
set(I8008_TEST_NAME ${I8008_LIB_NAME}_tests)

set(I8008_FILES
        src/Constants8008.h
        src/MemoryView.h
        src/Instructions8008.h src/Instructions8008.cpp
        src/Disassemble8008.h src/Disassemble8008.cpp
        )

set(I8008_TEST_FILES
        tests/disassemble_8008_test.cpp
        tests/instructions_8008_test.cpp
        )

add_library(${I8008_LIB_NAME} ${I8008_FILES})
target_include_directories(${I8008_LIB_NAME} PRIVATE ../)
target_link_libraries(${I8008_LIB_NAME} ${MISC_UTILS_LIB_NAME})
set_warnings(${I8008_LIB_NAME})

if(WITH_TESTS)
    add_executable(${I8008_TEST_NAME} ${I8008_TEST_FILES})
    set_target_properties(${I8008_TEST_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(${I8008_TEST_NAME} PRIVATE gtest gtest_main gmock PUBLIC ${I8008_LIB_NAME})
    target_include_directories(${I8008_TEST_NAME} PRIVATE src/)
    target_include_directories(${I8008_TEST_NAME} PRIVATE ../)
    if(${LINK_STATIC_EXE})
        target_link_options(${I8008_TEST_NAME} PRIVATE -static-libgcc -static-libstdc++)
    endif()

    add_test(NAME ${I8008_TEST_NAME} COMMAND ${I8008_TEST_NAME})
endif()