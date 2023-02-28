cmake_minimum_required(VERSION 3.13)

project(MicralN
        VERSION 0.1
        DESCRIPTION "An emulator for the Micral N"
        LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)

# Tools
function(set_warnings target)
    if(DEFINED ENV{MICRAL_WARNINGS})
        separate_arguments(WARNING_PARAMETERS NATIVE_COMMAND $ENV{MICRAL_WARNINGS})
        target_compile_options(${target} PRIVATE ${WARNING_PARAMETERS})
    else()
        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            message(STATUS "MICRAL_WARNINGS environment variable not found. Set default for Clang. (${target})")
            target_compile_options(${target} PRIVATE -Wall -Werror -Wno-unknown-pragmas -Wno-unused-variable)
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            message(STATUS "MICRAL_WARNINGS environment variable not found. Set default for GCC. (${target})")
            target_compile_options(${target} PRIVATE -Wall -Werror -Wno-unknown-pragmas -Wno-unused-variable)
        else()
            message(STATUS "MICRAL_WARNINGS environment variable not found. No specific warning setup done. (${target})")
        endif()

    endif()
endfunction()

# Platforms
if(WIN32 AND NOT MSVC)
    set(LINK_STATIC_EXE TRUE)
endif()

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2  -s USE_GLFW=3 -s USE_WEBGL2=1 -s FORCE_FILESYSTEM=1")

    set(WITH_LOGURU FALSE)
    set(WITH_TESTS FALSE)
    set(SDL_TARGET "")
    set(GLFW_TARGET "")
    set(GUI_LINKS "")

    set(COMPILE_CLI FALSE)
    set(COMPILE_DISASSEMBLER FALSE)
else()
    set(WITH_LOGURU TRUE)
    set(WITH_TESTS TRUE)
    set(SDL_TARGET SDL2::SDL2)
    set(GLFW_TARGET glfw)
    set(GUI_LINKS SDL2::SDL2main OpenGL::GL glfw)

    set(COMPILE_CLI TRUE)
    set(COMPILE_DISASSEMBLER TRUE)
endif()

set(MICRAL_PACKAGE_DIR ${PROJECT_SOURCE_DIR})
cmake_path(APPEND MICRAL_PACKAGE_DIR install_dir)
cmake_path(APPEND MICRAL_PACKAGE_DIR ${CMAKE_SYSTEM_NAME})

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

# Add the cmake/ local folders
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
if(SDL_TARGET)
    add_subdirectory(extern/SDL)
endif()
find_package(OpenGL REQUIRED)

if(${OPENGL_FOUND})
    set(GUI_ENABLED TRUE)
endif()

# Google Test
if(WITH_TESTS)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(extern/googletest)
endif()

# Threads
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# Loguru
if(WITH_LOGURU)
    set(LOGURU_LIB_NAME "loguru")
    set(LOGURU_FILES extern/loguru/loguru.cpp)

    add_library(${LOGURU_LIB_NAME} ${LOGURU_FILES})
    target_include_directories(${LOGURU_LIB_NAME} PUBLIC extern/loguru/)
    target_link_libraries(${LOGURU_LIB_NAME} INTERFACE Threads::Threads)
endif()

# DearImGUI
if(${GUI_ENABLED})
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
    target_link_libraries(${DEARIMGUI_LIB_NAME} PUBLIC ${SDL_TARGET})

    if(WIN32 AND NOT MSVC)
        target_link_libraries(${DEARIMGUI_LIB_NAME} INTERFACE imm32)
    endif()
endif()

# CLI11
add_subdirectory(extern/CLI11)

# GLFW
if(${GUI_ENABLED} AND GLFW_TARGET)
    add_subdirectory(extern/glfw)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

# Project Structure
set(EMULATION_LIB_NAME "emulation_core")
set(DEVICES_LIB_NAME "devices")
set(EMULATOR_LIB_NAME "emulator")
set(I8008_LIB_NAME "i8008")
set(FILE_UTILS_LIB_NAME "file_utils")
set(MISC_UTILS_LIB_NAME "misc_utils")

add_subdirectory(src/file_utils)
add_subdirectory(src/misc_utils)
add_subdirectory(src/i8008)
add_subdirectory(src/devices)
add_subdirectory(src/emulator)
add_subdirectory(src/emulation_core)

if(COMPILE_CLI)
    add_subdirectory(src/cli)
endif()

if(COMPILE_DISASSEMBLER)
    add_subdirectory(src/disassembler)
endif()

if(${GUI_ENABLED})
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

if(NOT EMSCRIPTEN)
    install(TARGETS SDL2
            DESTINATION ${MICRAL_PACKAGE_DIR}
            RUNTIME
            CONFIGURATIONS Release
            )

endif()

if(WIN32 AND NOT MSVC)
    # Manually copy the required files
    # A bit of fiddling here I must confess.
    # Not really elegant.
    find_file(WIN_THREAD_LIB_FILE "libwinpthread-1.dll"
            PATHS ${CMAKE_FIND_ROOT_PATH}
            PATH_SUFFIXES lib
            NO_CACHE
            REQUIRED)
    install(FILES ${WIN_THREAD_LIB_FILE}
            DESTINATION ${MICRAL_PACKAGE_DIR}
            CONFIGURATIONS Release
            )

    find_file(WIN_GCC_LIB_FILE "libgcc_s_sjlj-1.dll" "libstdc++-6.dll"
            PATHS ${CMAKE_FIND_ROOT_PATH}
            HINTS ${CMAKE_FIND_ROOT_PATH}
            CMAKE_FIND_ROOT_PATH_BOTH
            NO_CACHE
            REQUIRED)
    install(FILES ${WIN_GCC_LIB_FILE}
            DESTINATION ${MICRAL_PACKAGE_DIR}
            CONFIGURATIONS Release
            )

    find_file(WIN_CPP_LIB_FILE "libstdc++-6.dll"
            PATHS ${CMAKE_FIND_ROOT_PATH}
            HINTS ${CMAKE_FIND_ROOT_PATH}
            CMAKE_FIND_ROOT_PATH_BOTH
            NO_CACHE
            REQUIRED)
    install(FILES ${WIN_CPP_LIB_FILE}
            DESTINATION ${MICRAL_PACKAGE_DIR}
            CONFIGURATIONS Release
            )

endif()

install(DIRECTORY ${PROJECT_SOURCE_DIR}/data
        DESTINATION ${MICRAL_PACKAGE_DIR}
        CONFIGURATIONS Release
        FILES_MATCHING PATTERN "*.bin"
        )
