cmake_minimum_required(VERSION 3.16)

project(micraln-gui
        DESCRIPTION "The GUI interface for the Micral N emulation"
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

# GUI
set(GUI_EXE_NAME "micral_gui")
set(GUI_TEST_EXE_NAME ${GUI_EXE_NAME}_tests)

set(GUI_FILES src/main.cpp
        ../../extern/imgui/backends/imgui_impl_opengl2.cpp
        ../../extern/imgui/backends/imgui_impl_sdl.cpp)
set(GUI_TEST_FILES tests/dummy_test.cpp)

add_executable(${GUI_EXE_NAME} ${GUI_FILES})
set_target_properties(${GUI_EXE_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${GUI_EXE_NAME} ${EMULATION_LIB_NAME} ${DEVICES_LIB_NAME} SDL2 OpenGL glfw ${DEARIMGUI_LIB_NAME})
target_include_directories(${GUI_EXE_NAME} PRIVATE ${SDL2_INCLUDE_DIRS}) # Because of the ImGui Backend uses #include <SDL.h> directly
target_include_directories(${GUI_EXE_NAME} PRIVATE ../)

if(LTO_result AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
    set_target_properties(${GUI_EXE_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    message("IPO activated for ${GUI_EXE_NAME}")
endif()

add_executable(${GUI_TEST_EXE_NAME} ${GUI_TEST_FILES})
target_link_libraries(${GUI_TEST_EXE_NAME} ${EMULATION_LIB_NAME} ${DEVICE_LIB_NAME} gtest gtest_main)

add_test(NAME ${GUI_TEST_EXE_NAME} COMMAND ${GUI_TEST_EXE_NAME})
