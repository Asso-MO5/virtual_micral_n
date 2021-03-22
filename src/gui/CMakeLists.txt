cmake_minimum_required(VERSION 3.13)

project(micraln-gui
        DESCRIPTION "The GUI interface for the Micral N emulation"
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

# GUI
set(GUI_EXE_NAME "micral_gui")
set(GUI_LIB_NAME ${GUI_EXE_NAME}_lib)
set(GUI_TEST_EXE_NAME ${GUI_EXE_NAME}_tests)

set(GUI_FILES src/main.cpp
        ../../extern/imgui/backends/imgui_impl_opengl2.cpp
        ../../extern/imgui/backends/imgui_impl_sdl.cpp
        src/ImGuiSDLGLContext.cpp src/ImGuiSDLGLContext.h
        src/ControllerWidget.cpp src/ControllerWidget.h
        src/SignalPlot.cpp src/SignalPlot.h
        src/panels/Panel8008.cpp src/panels/Panel8008.h
        src/panels/PanelDisassembly.cpp src/panels/PanelDisassembly.h
        src/panels/PanelMemory.cpp src/panels/PanelMemory.h
        src/panels/PanelControl.cpp src/panels/PanelControl.h
        src/widgets/PanelSwitch.cpp src/widgets/PanelSwitch.h
        src/widgets/PanelLed.cpp src/widgets/PanelLed.h
        src/panels/PanelPluribus.cpp src/panels/PanelPluribus.h
        src/panels/PanelSignals.cpp src/panels/PanelSignals.h
        src/ValuePlot.cpp src/ValuePlot.h
        src/Plot.cpp src/Plot.h
        src/PlotStructures.h)

set(GUI_FILES_LIB src/lib/Averager.cpp src/lib/Averager.h)

set(GUI_TEST_FILES tests/averager_test.cpp)

add_library(${GUI_LIB_NAME} ${GUI_FILES_LIB})
target_include_directories(${GUI_LIB_NAME} PRIVATE ../)

add_executable(${GUI_EXE_NAME} ${GUI_FILES})
set_target_properties(${GUI_EXE_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${GUI_EXE_NAME} ${EMULATOR_LIB_NAME} ${GUI_LIB_NAME} SDL2 OpenGL glfw ${DEARIMGUI_LIB_NAME})
target_include_directories(${GUI_EXE_NAME} PRIVATE ${SDL2_INCLUDE_DIRS}) # Because of the ImGui Backend uses #include <SDL.h> directly
target_include_directories(${GUI_EXE_NAME} PRIVATE ../)

if(LTO_result AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
    set_target_properties(${GUI_EXE_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    message("IPO activated for ${GUI_EXE_NAME}")
endif()


add_executable(${GUI_TEST_EXE_NAME} ${GUI_TEST_FILES})
target_link_libraries(${GUI_TEST_EXE_NAME} ${DEVICE_LIB_NAME} ${EMULATION_LIB_NAME} ${GUI_LIB_NAME} gmock gtest_main)
target_include_directories(${GUI_TEST_EXE_NAME} PRIVATE src/lib/)
target_include_directories(${GUI_TEST_EXE_NAME} PRIVATE ../)

add_test(NAME ${GUI_TEST_EXE_NAME} COMMAND ${GUI_TEST_EXE_NAME})
