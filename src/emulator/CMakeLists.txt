cmake_minimum_required(VERSION 3.13)

project(micraln-emulator
        DESCRIPTION "The emulator for the Micral N emulation"
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

# Emulator
set(EMULATOR_TEST_NAME ${EMULATOR_LIB_NAME}_tests)

set(EMULATOR_FILES src/SimulationRunType.h
        src/Simulator.cpp src/Simulator.h
        src/SignalRecorder.cpp src/SignalRecorder.h
        src/ValueRecorder.cpp src/ValueRecorder.h
        src/Recorder.h src/Recorder.cpp
        src/RecorderCollection.cpp src/RecorderCollection.h
        src/VirtualDisk.cpp src/VirtualDisk.h
        src/VirtualTTY.cpp src/VirtualTTY.h)

set(EMULATOR_TEST_FILES
        tests/signal_recorder_test.cpp
        tests/value_recorder_test.cpp
        tests/virtual_disk_test.cpp
        tests/virtual_tty_test.cpp)

add_library(${EMULATOR_LIB_NAME} ${EMULATOR_FILES})
target_include_directories(${EMULATOR_LIB_NAME} PRIVATE ../)
target_link_libraries(${EMULATOR_LIB_NAME} PUBLIC ${DEVICES_LIB_NAME} ${EMULATION_LIB_NAME} ${FILE_UTILS_LIB_NAME})
set_warnings(${EMULATOR_LIB_NAME})

add_executable(${EMULATOR_TEST_NAME} ${EMULATOR_TEST_FILES})
set_target_properties(${EMULATOR_TEST_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${EMULATOR_TEST_NAME} gtest gtest_main gmock ${EMULATOR_LIB_NAME})
target_include_directories(${EMULATOR_TEST_NAME} PRIVATE src/)
target_include_directories(${EMULATOR_TEST_NAME} PRIVATE ../)

add_test(NAME ${EMULATOR_TEST_NAME} COMMAND ${EMULATOR_TEST_NAME})
