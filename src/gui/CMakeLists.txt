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
        )

set(GUI_FILES_LIB
        src/lib/Averager.cpp src/lib/Averager.h
        src/ControllerWidget.cpp src/ControllerWidget.h
        src/ImGuiSDLGLContext.cpp src/ImGuiSDLGLContext.h
        src/Plot.cpp src/Plot.h
        src/PlotStructures.h
        src/SignalPlot.cpp src/SignalPlot.h
        src/ValuePlot.cpp src/ValuePlot.h
        src/panels/Panel8008.cpp src/panels/Panel8008.h
        src/panels/PanelControl.cpp src/panels/PanelControl.h
        src/panels/PanelDisassembly.cpp src/panels/PanelDisassembly.h
        src/panels/PanelDiskController.cpp src/panels/PanelDiskController.h
        src/panels/PanelMemory.cpp src/panels/PanelMemory.h
        src/panels/PanelPluribus.cpp src/panels/PanelPluribus.h
        src/panels/PanelProcessorCard.cpp src/panels/PanelProcessorCard.h
        src/panels/PanelSignals.cpp src/panels/PanelSignals.h
        src/panels/PanelStackChannelCard.cpp src/panels/PanelStackChannelCard.h
        src/widgets/PanelLed.cpp src/widgets/PanelLed.h
        src/widgets/PanelMemoryDisplay.h
        src/widgets/PanelSwitch.cpp src/widgets/PanelSwitch.h
        src/panels/PanelTTY.cpp src/panels/PanelTTY.h)

set(GUI_TEST_FILES tests/averager_test.cpp)

add_library(${GUI_LIB_NAME} ${GUI_FILES_LIB})
target_include_directories(${GUI_LIB_NAME} PRIVATE ../)
target_link_libraries(${GUI_LIB_NAME} PUBLIC
        ${DEVICE_LIB_NAME}
        ${EMULATOR_LIB_NAME}
        ${DEARIMGUI_LIB_NAME})

set(MINGW_LIB "")
if(WIN32 AND NOT MSVC)
    set(MINGW_LIB mingw32)
endif()

add_executable(${GUI_EXE_NAME} ${GUI_FILES})
set_target_properties(${GUI_EXE_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${GUI_EXE_NAME} ${EMULATOR_LIB_NAME} ${GUI_LIB_NAME}
        ${MINGW_LIB}
        SDL2::SDL2main
        OpenGL::GL glfw)
target_include_directories(${GUI_EXE_NAME} PRIVATE ../)
set_warnings(${GUI_EXE_NAME})
if(${LINK_STATIC_EXE})
    target_link_options(${GUI_EXE_NAME} PRIVATE -static-libgcc -static-libstdc++)
endif()


if(LTO_result AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
    set_target_properties(${GUI_EXE_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    message(STATUS "IPO activated for ${GUI_EXE_NAME}")
endif()


add_executable(${GUI_TEST_EXE_NAME} ${GUI_TEST_FILES})
target_link_libraries(${GUI_TEST_EXE_NAME}  ${GUI_LIB_NAME} gmock gtest_main)
target_include_directories(${GUI_TEST_EXE_NAME} PRIVATE src/lib/)
target_include_directories(${GUI_TEST_EXE_NAME} PRIVATE ../)
if(${LINK_STATIC_EXE})
    target_link_options(${GUI_TEST_EXE_NAME} PRIVATE -static-libgcc -static-libstdc++)
endif()

add_test(NAME ${GUI_TEST_EXE_NAME} COMMAND ${GUI_TEST_EXE_NAME})
