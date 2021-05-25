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

The CMAKE reacts to these environment variables:

* MICRAL_WARNINGS sets up the warnings for compilation. Recommended value is:
    * For GCC: MICRAL_WARNINGS="-Wall -Werror -Wno-unused-but-set-variable -Wno-unknown-pragmas -Wno-unused-variable"
    * For Clang: MICRAL_WARNINGS="-Wall -Werror -Wno-unknown-pragmas -Wno-unused-variable"
    * For Windows: not tested yet

Reasons for exceptions:

* unused-but-set-variable is some asserts, could be resolved with debug only blocks
* no-unused-variable, because of string constants not used on the CLI, could be resolved by spliting the headers
* no-unknown-pragmas, because some pragmas are used for static analysis

The executable are generated in

```shell
src/cli/micral_cli
src/cli/micral_gui
```

### Static analysis

The CMake project prepares a `ANALYZE_CPPCHECK` target for the `Makefile` and `Ninja`
generator. Passing cppcheck is not mandatory yet, but could be in the future.

To analyze the project with Clang Tidy, you can use this :

``cmake -DCMAKE_CXX_CLANG_TIDY="clang-tidy;-checks=-*,clang-analyzer-cplusplus*,cppcoreguidelines-*,modernize-*,portability-,*readability-*" ..``

Passing clang-tidy is not mandatory at the moment (it's too global and gives warning about externals, and is
not well tuned for the project).

## Build for Clion

**TODO**

## Build for Visual Studio Code

**TODO**

## Build for Visual Studio

**TODO**

## Externals

The external dependencies are located in the `extern/` sub-folder and rely on git submodules.

### Not using Git

If the sources are used out of a Git environment (offline for example), the needed library sources can be installed in
the `extern/` sub-folder directly.

The needed library sources are:

* Google Test (including Google Mock), in the `googletest` folder
* DearImGui, in the `imgui` folder.
