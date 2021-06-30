cmake_minimum_required(VERSION 3.13)

project(micraln-devices
        DESCRIPTION "The devices for the Micral N emulation"
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

# Devices
set(DEVICES_TEST_NAME ${DEVICES_LIB_NAME}_tests)

set(DEVICES_FILES
        src/Clock.cpp src/Clock.h
        src/CPU8008.cpp src/CPU8008.h
        src/DoubleClock.cpp src/DoubleClock.h
        src/AddressStack.cpp src/AddressStack.h
        src/AutomaticStart.cpp src/AutomaticStart.h
        src/InterruptController.cpp src/InterruptController.h
        src/ConsoleCard.cpp src/ConsoleCard.h
        src/Pluribus.cpp src/Pluribus.h
        src/ProcessorCard.cpp src/ProcessorCard.h
        src/MemoryCard.cpp src/MemoryCard.h
        src/PluribusConfiguration.cpp src/PluribusConfiguration.h
        src/InterruptCircuit.cpp src/InterruptCircuit.h
        src/GeneralAddressRegister.cpp src/GeneralAddressRegister.h
        src/StackChannelCard.cpp src/StackChannelCard.h
        src/DataOnMDBusHolder.cpp src/DataOnMDBusHolder.h
        src/IOCard.cpp src/IOCard.h
        src/DiskControllerCard.cpp src/DiskControllerCard.h
        src/connectors/IO_To_StackChannel.cpp src/connectors/IO_To_StackChannel.h
        src/connectors/IO_To_DiskController.cpp src/connectors/IO_To_DiskController.h
        src/connectors/StackChannel_To_DiskController.cpp src/connectors/StackChannel_To_DiskController.h)

set(DEVICES_TEST_FILES
        tests/clock_test.cpp
        tests/8008_test.cpp
        tests/double_clock_test.cpp
        tests/address_stack_test.cpp
        tests/interrupt_circuit_test.cpp
        )

add_library(${DEVICES_LIB_NAME} ${DEVICES_FILES})
target_include_directories(${DEVICES_LIB_NAME} PRIVATE ../)
set_warnings(${DEVICES_LIB_NAME})
target_link_libraries(${DEVICES_LIB_NAME} INTERFACE ${I8008_LIB_NAME})

add_executable(${DEVICES_TEST_NAME} ${DEVICES_TEST_FILES})
set_target_properties(${DEVICES_TEST_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${DEVICES_TEST_NAME}
        PRIVATE gtest gtest_main gmock
        PUBLIC ${DEVICES_LIB_NAME} ${EMULATION_LIB_NAME} ${I8008_LIB_NAME})
target_include_directories(${DEVICES_TEST_NAME} PRIVATE src/)
target_include_directories(${DEVICES_TEST_NAME} PRIVATE ../)

add_test(NAME ${DEVICES_TEST_NAME} COMMAND ${DEVICES_TEST_NAME})
