# Electric Driven Baby Car Control Firmware

## Important thing to know

* This firmware is using **CMake** based build system.
* This firmware is designed to run on **Arduino Leonardo** (or analog) boards.
* This instructions is only for **CLion** running under **macOS**.

## How to build

1. Clone [Arduino-CMake-NG](https://github.com/arduino-cmake/Arduino-CMake-NG) repository.
1. Set `ARDUINO_CMAKE_TOOLCHAIN_PATH` environment variable with the cloned repository path (it should be something like
`$HOME/Github/arduino-cmake/Arduino-CMake-NG`). 
1. Locate **AVR Tools** (on macOS it is most likely located in 
`/Applications/Arduino.app/Contents/Java/hardware/tools/avr`)
1. Set `ARDUINO_AVR_PATH` environment variable with absolute path to **AVR Tools** directory.
1. Locate your connected **Arduino Leonardo** board port (it should be something like `/dev/cu.usbmodem145101`).
1. Set `UPLOAD_PORT` environment variable in CMake settings to board port you've just found out.
1. Follow [toolchain manual](https://github.com/arduino-cmake/Arduino-CMake-NG/wiki/Creating-First-Program) 
and pass the following argument to CMake: `-DCMAKE_TOOLCHAIN_FILE=${ARDUINO_CMAKE_TOOLCHAIN_PATH}/Arduino-Toolchain.cmake`.

## Arduino-CMake-NG

This project is using [Arduino-CMake-NG](https://github.com/arduino-cmake/Arduino-CMake-NG) toolchain o build and upload 
firmware to target device. In fact upload is kind of broken for Leonardo class of devices but workaround is implemented.

### How to set environment variable globally
On macOS you should modify your terminal profile to keep your environment variables set in any new terminal session.

This can be done by adding this line `export SOME_ENV_VALUE="/Some/Value/Meaning"` to `~/.bash_profile` or `~/.zshrc` 
depending on terminal you are using.

You should restart terminal session (or any other application) for the changes to take effect.

### How to set environment variable in CMake settings of CLion

Open _Preferences_ > _Build, Execution, Deployment_ > _CMake_.

Edit _Environment_ field.
 