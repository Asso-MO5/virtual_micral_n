# Build

The build system is relying on CMake. The entry point is the `CMakeLists.txt` at the root of the project tree.

## Build command in shell

Prerequisites:

* CMake 3.16+
* C++ Compiler with C++17 support
* Git (with possible workaround)
  
For the GUI:
* SDL2 development files
    * sudo apt install libsdl2-dev
* OpenGL development files
    * sudo apt install libopengl-dev
    * sudo apt install libglfw3-dev


```shell
mkdir build
cd build
cmake ../ # or cmake ../ -DCMAKE_BUILD_TYPE=Debug
make
```

The executable are generated in
```shell
src/cli/micral_cli
src/cli/micral_gui
```

## Build for Clion

**TODO**

## Build for Visual Studio Code

**TODO**

## Build for Visual Studio

**TODO**

## Externals

The external dependencies are located in the `extern/` sub-folder and rely on git submodules.

### Not using Git

If the sources are used out of a Git environment (offline for example), the needed library sources can
be installed in the `extern/` sub-folder directly.

The needed library sources are:

  * Google Test (including Google Mock), in the `googletest` folder
  * DearImGui, in the `imgui` folder.
