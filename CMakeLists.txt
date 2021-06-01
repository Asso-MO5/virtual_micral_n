cmake_minimum_required(VERSION 3.13)

project(MicralN
        VERSION 0.1
        DESCRIPTION "An emulator for the Micral N"
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

# Tools
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

function(set_warnings target)
    if(DEFINED ENV{MICRAL_WARNINGS})
        separate_arguments(WARNING_PARAMETERS NATIVE_COMMAND $ENV{MICRAL_WARNINGS})
        target_compile_options(${target} PRIVATE ${WARNING_PARAMETERS})
    else()
        message(STATUS "MICRAL_WARNINGS environment variable not found. No specific warning setup done.")
    endif()
endfunction()

# Avoid extensions
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

# Default build type if not specified
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Add the cmake/ local folder
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Prepare for LTO
include(CheckIPOSupported)
check_ipo_supported(RESULT LTO_result OUTPUT output)

if(NOT LTO_result)
    message(WARNING "IPO is not supported: ${output}")
endif()

# Submodules
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)

    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/googletest/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

# SDL2 / OpenGL
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

# Google Test
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(extern/googletest)
set_target_properties(gtest PROPERTIES CXX_CLANG_TIDY "")
set_target_properties(gtest_main PROPERTIES CXX_CLANG_TIDY "")

# Threads
# set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# Loguru
set(LOGURU_LIB_NAME "loguru")
set(LOGURU_FILES extern/loguru/loguru.cpp)

add_library(${LOGURU_LIB_NAME} ${LOGURU_FILES})
target_include_directories(${LOGURU_LIB_NAME} PUBLIC extern/loguru/)
target_link_libraries(${LOGURU_LIB_NAME} INTERFACE Threads::Threads dl)
set_target_properties(${LOGURU_LIB_NAME} PROPERTIES CXX_CLANG_TIDY "")

# DearImGUI
set(DEARIMGUI_LIB_NAME "dearimgui")
set(DEARIMGUI_FILES
        extern/imgui/imgui.cpp extern/imgui/imgui.h
        extern/imgui/imgui_demo.cpp
        extern/imgui/imgui_draw.cpp
        extern/imgui/imgui_tables.cpp
        extern/imgui/imgui_widgets.cpp)

add_library(${DEARIMGUI_LIB_NAME} ${DEARIMGUI_FILES})
target_include_directories(${DEARIMGUI_LIB_NAME} PUBLIC extern/imgui/)
target_include_directories(${DEARIMGUI_LIB_NAME} INTERFACE extern/imgui/backends)
set_target_properties(${DEARIMGUI_LIB_NAME} PROPERTIES CXX_CLANG_TIDY "" CMAKE_CXX_CPPCHECK "")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

# Project Structure
set(EMULATION_LIB_NAME "emulation_core")
set(DEVICES_LIB_NAME "devices")
set(EMULATOR_LIB_NAME "emulator")
set(MCS8_LIB_NAME "mcs8")

add_subdirectory(src/mcs8)
add_subdirectory(src/devices)
add_subdirectory(src/emulator)
add_subdirectory(src/emulation_core)
add_subdirectory(src/cli)

if(${SDL2_FOUND} AND ${OPENGL_FOUND})
    message(STATUS "Compiling GUI")

    set(IMGUI_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/extern/imgui")
    add_subdirectory(src/gui)
else()
    message(STATUS "Missing dependencies for GUI. Excluding it.")
    if(NOT ${SDL2_FOUND})
        message(WARNING "SDL2 not present")
    endif()
    if(NOT ${OPENGL_FOUND})
        message(WARNING "OpenGL not present")
    endif()
endif()


# Prepare CppCheck
# Done at the end to avoid automatic cppcheck on each individual file
if(UNIX)
    # Configuration not done yet for other platforms.
    find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
    if(CMAKE_CXX_CPPCHECK)
        message(STATUS "Configuring cppcheck.")
        list(
                APPEND CMAKE_CXX_CPPCHECK
                "--enable=all"
                "--inconclusive"
                "--error-exitcode=1"
                "--std=c++20"
                "--template=gcc"
                #            "--force"
                "--suppressions-list=${CMAKE_SOURCE_DIR}/CppCheckSuppression.txt"
                "-iextern"
                "src/"
        )

        add_custom_target(ANALYZE_CPPCHECK DEPENDS micral_cli micral_gui
                COMMAND ${CMAKE_CXX_CPPCHECK}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "Static code analysis using ${CPP_CHECK_VERSION}"
                )
    endif()
endif()

