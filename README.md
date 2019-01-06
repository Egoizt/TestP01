# Electric Driven Baby Car Control Firmware

## How to build

**Important thing to know**
* This firmware is using CMake based build system
* This firmware relies on [Arduino-CMake-NG](https://github.com/arduino-cmake/Arduino-CMake-NG) toolchain
* This firmware is designed to run on Arduino Leonardo (or analog) boards

1. Clone https://github.com/arduino-cmake/Arduino-CMake-NG repository
1. Set `ARDUINO_CMAKE_TOOLCHAIN_PATH` environment variable to the cloned repository path. 
   1. On macOS this can be done by adding this line 
   `ARDUINO_CMAKE_TOOLCHAIN_PATH="$HOME/Github/arduino-cmake/Arduino-CMake-NG"` 
   to `~/.bash_profile`. 
1. Following [toolchain manual](https://github.com/arduino-cmake/Arduino-CMake-NG/wiki/Creating-First-Program) 
pass the following argument to CMake: `-DCMAKE_TOOLCHAIN_FILE=${ARDUINO_CMAKE_TOOLCHAIN_PATH}/Arduino-Toolchain.cmake`.
