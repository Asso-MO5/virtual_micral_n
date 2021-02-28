cmake_minimum_required(VERSION 3.16)

project(micraln-emulation-core
        DESCRIPTION "The emulation core for the Micral N emulation"
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

# Emulation Core
set(EMULATION_TEST_NAME ${EMULATION_LIB_NAME}_tests)

set(EMULATION_CORE_FILES
        src/Schedulable.h
        src/Scheduling.h
        src/Scheduler.h src/Scheduler.cpp
        src/Edge.h
        src/Frequency.h
        src/State.h src/State.cpp
        src/ConnectedData.cpp src/ConnectedData.h
        src/DataBus.cpp src/DataBus.h
        src/OwnedSignal.cpp src/OwnedSignal.h
        src/OwnedValue.h)

set(EMULATION_CORE_TEST_FILES
        tests/scheduler_test.cpp
        tests/edge_test.cpp
        tests/state_test.cpp
        tests/databus_test.cpp
        tests/connecteddata_test.cpp
        tests/ownedsignal_test.cpp
        tests/ownedvalue_test.cpp)

add_library(${EMULATION_LIB_NAME} ${EMULATION_CORE_FILES})

add_executable(${EMULATION_TEST_NAME} ${EMULATION_CORE_TEST_FILES})
set_target_properties(${EMULATION_TEST_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${EMULATION_TEST_NAME} gtest gtest_main gmock ${EMULATION_LIB_NAME})
target_include_directories(${EMULATION_TEST_NAME} PRIVATE src/)
target_include_directories(${EMULATION_TEST_NAME} PRIVATE ../)

add_test(NAME ${EMULATION_TEST_NAME} COMMAND ${EMULATION_TEST_NAME})
