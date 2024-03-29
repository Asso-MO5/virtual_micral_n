# Build

The build system is relying on CMake. The entry point is the `CMakeLists.txt` at the root of the project tree.

## Build for Ubuntu with command line

*Tested with Ubuntu 20.04*

Prerequisites:

* CMake 3.16+
* C++ Compiler with C++20 support
* Git (with possible workaround)

For the GUI:

* SDL2 development files
    * On Ubuntu: ```sudo apt install libsdl2-dev```
    * In fact, a local SDL2 library is compiled. Installing the developement package is an easy way to have all the
      necessary dependant libraries install though, as they are numerous.

* OpenGL development files
    * On Ubuntu:

```
      sudo apt install libopengl-dev
````

To build manually in *Debug*:

```shell
mkdir build
cd build
cmake ../ # or cmake ../ -DCMAKE_BUILD_TYPE=Debug
make
```

The CMAKE reacts to these environment variables:

* You can override default warning settings by using the MICRAL_WARNINGS environment variable. Recommended value is:
    * For GCC: MICRAL_WARNINGS="-Wall -Werror -Wno-unknown-pragmas -Wno-unused-variable"
    * For Clang: MICRAL_WARNINGS="-Wall -Werror -Wno-unknown-pragmas -Wno-unused-variable"
    * For other compilers: not tested yet
* If not set, default settings are applied

Reasons for exceptions:

* no-unused-variable, because of string constants not used on the CLI, could be resolved by spliting the headers
* no-unknown-pragmas, because some pragmas are used for static analysis

The executable are generated in

```shell
src/cli/micral_cli
src/gui/micral_gui
src/disassembler/disassembler
```

## Build for MacOS with command line

*Tested with MacOS Big Sur*

Prerequisites:

* CMake 3.16+
* CLang Compiler
* Git (with possible workaround)

To build manually :

```shell
mkdir build
cd build
cmake ../ # or cmake ../ -DCMAKE_BUILD_TYPE=Debug
make
```

The CMAKE reacts to these environment variables:

* You can override default warning settings by using the MICRAL_WARNINGS environment variable. Recommended value is:
  * For Clang: MICRAL_WARNINGS="-Wall -Werror -Wno-unknown-pragmas -Wno-unused-variable"
  * For other compilers: not tested yet
* If not set, default settings are applied

Reasons for exceptions:

* no-unused-variable, because of string constants not used on the CLI, could be resolved by spliting the headers
* no-unknown-pragmas, because some pragmas are used for static analysis

The executable are generated in

```shell
build/src/cli/micral_cli
build/src/gui/micral_gui
build/src/disassembler/disassembler
```

## Build for Emscripten on Ubuntu

* Install Emscripten.
* Source the environment installation script.
* Launch build from the project folder:

```shell
mkdir build_js
cd build_js
encmake cmake ../ # or cmake ../ -DCMAKE_BUILD_TYPE=Debug
make
```

* To get the files to deploy, see Packaging further in this document.
    - Test in a web server : python -m http.server in Python 3 or python -m SimpleHTTPServer in Python 2
    - Access it at : http://localhost:8000/

## Build for Emscripten on MacOS

* Install Emscripten

```shell
brew install emscripten
```

* Launch build from the project folder:

```shell
mkdir build_js
cd build_js
emcmake cmake ../ # or cmake ../ -DCMAKE_BUILD_TYPE=Debug
make
```

* You can override default warning settings by using the MICRAL_WARNINGS environment variable. Recommended value is:
  * For Clang: MICRAL_WARNINGS="-Wall -Werror -Wno-unknown-pragmas -Wno-unused-variable"
* If not set, default settings are applied

* To get the files to deploy, see Packaging further in this document.
    - Test in a web server : python -m http.server in Python 3 or python -m SimpleHTTPServer in Python 2
    - Access it at : http://localhost:8000/

## Cross build for Windows from Ubuntu 20.04

* `sudo apt install mingw-w64`
* Create the CMake project with ` -DCMAKE_TOOLCHAIN_FILE=ubuntu-mingw64.cmake `
  and the `CMAKE_BUILD_TYPE` of your choice
* You might need also to pass `-DSDL_SHARED=ON`.

## Build for Clion

Open the root folder of the project and let Clion setup the project according to the CMake configuration.

You can then add the different targets you want to compile with.

**TODO**: complete the CMakePresets.json

## Build for Visual Studio

The software is not ported to Windows yet. It has some things missing or to adapt before it can.

* Need to install the development files for SDL2.
* Need to configure the CMake file to find them.
* Need to manage the unknown #pragma's used for diagnostics.
* There's a dependency on 'dl' that seems unnecessary (or even nonexistent).

**TODO**

## Other operations

### Static analysis

To analyze the project with CppCheck, you can use this :

    cppcheck --enable=all --inconclusive --error-exitcode=1 --std=c++20 --template=gcc --suppressions-list=CppCheckSuppression.txt -iextern src/ # Works as intented

To analyze the project with Clang Tidy, you can use this :

    cmake -DCMAKE_CXX_CLANG_TIDY="clang-tidy;-checks=-*,clang-analyzer-cplusplus*,cppcoreguidelines-*,modernize-*,portability-,*readability-*" ..

Passing clang-tidy is not mandatory at the moment (it's too global and gives warning about externals, and is not well
tuned for the project).

### Dependency viewer

You can generate a module dependency view with `cmake --graphviz=deps.dot ..` in a build directory.

Turn into a SVG file with `dot -Tsvg -o deps.dot.svg deps.dot`. You need [graphviz](https://graphviz.org/) command `dot`
for this.

### Packaging

`make install` can be used to place the needed files for a package in the `install_dir`
folder at project's root.

There's a problem with SDL2 trying to install its tools somewhere. At the moment, a way to allow it to be placed
somewhere we don't care of is to use `-DCMAKE_INSTALL_PREFIX` to point somewhere.

## Externals

The external dependencies are located in the `extern/` sub-folder and rely on *git submodules*.

### Not using Git

If the sources are used out of a Git environment (offline for example), the needed library sources can be installed in
the `extern/` sub-folder directly.

The needed library sources are:

* Google Test (including Google Mock), in the `googletest` folder
* DearImGui, in the `imgui` folder.
* loguru, in the `loguru` folder.
