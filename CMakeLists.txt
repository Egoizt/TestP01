cmake_minimum_required(VERSION 3.8)

project(TestP01 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_VERBOSE_MAKEFILE ON)

# This is a fix for PID and VID is missing for Leaonardo
# Taken from https://github.com/queezythegreat/arduino-cmake/issues/54
add_definitions(-DUSB_VID=0x2341)
add_definitions(-DUSB_PID=0x8036)

get_board_id(board_id leonardo)

add_arduino_executable(${PROJECT_NAME} ${board_id} src/main.cpp)
link_platform_library(${PROJECT_NAME} SPI ${board_id})

# Doesn't work due to uninvestigated configuration problem.
# Code below is workaround. Custom target to Upload to target device.
#upload_arduino_target(${PROJECT_NAME} ${board_id} $ENV{UPLOAD_PORT})

add_custom_target(
        Upload ALL
        DEPENDS ${PROJECT_NAME}
)

add_custom_command(
        TARGET  Upload
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        COMMAND stty -f $ENV{UPLOAD_PORT} ispeed 1200 ospeed 1200
        COMMAND sleep 2 #This one forces stty to finish work before avrdude comes in
        COMMAND $ENV{ARDUINO_AVR_PATH}/bin/avrdude -C$ENV{ARDUINO_AVR_PATH}/etc/avrdude.conf -patmega32u4 -cavr109 -v -v -v -v -P$ENV{UPLOAD_PORT} -b57600 -D -Uflash:w:${PROJECT_NAME}.hex:i
)
