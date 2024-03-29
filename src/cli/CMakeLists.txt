cmake_minimum_required(VERSION 3.13)

project(micraln-cli
        DESCRIPTION "The command line interface for the Micral N emulation"
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

# CLI
set(CLI_EXE_NAME "micral_cli")
set(CLI_TEST_EXE_NAME ${CLI_EXE_NAME}_tests)

set(CLI_FILES src/main.cpp
        src/TextInput.cpp src/TextInput.h)
set(CLI_TEST_FILES tests/test_main.cpp)

add_executable(${CLI_EXE_NAME} ${CLI_FILES})
set_target_properties(${CLI_EXE_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${CLI_EXE_NAME} ${LOGURU_LIB_NAME} ${EMULATOR_LIB_NAME} Threads::Threads ${CMAKE_DL_LIBS})
target_include_directories(${CLI_EXE_NAME} PRIVATE ../)

if(${LINK_STATIC_EXE})
    target_link_options(${CLI_EXE_NAME} PRIVATE -static-libgcc -static-libstdc++)
endif()

if(LTO_result AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
    set_target_properties(${CLI_EXE_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    message(STATUS "IPO activated for ${CLI_EXE_NAME}")
endif()

set_warnings(${CLI_EXE_NAME})

install(TARGETS ${CLI_EXE_NAME}
        DESTINATION ${MICRAL_PACKAGE_DIR}
        CONFIGURATIONS Release
        )

if(WITH_TESTS)
    add_executable(${CLI_TEST_EXE_NAME} ${CLI_TEST_FILES})
    target_link_libraries(${CLI_TEST_EXE_NAME} ${DEVICES_LIB_NAME} ${EMULATION_LIB_NAME} gtest gtest_main)
    if(${LINK_STATIC_EXE})
        target_link_options(${CLI_TEST_EXE_NAME} PRIVATE -static-libgcc -static-libstdc++)
    endif()
    add_test(NAME ${CLI_TEST_EXE_NAME} COMMAND ${CLI_TEST_EXE_NAME})
endif()