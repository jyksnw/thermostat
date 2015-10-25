cmake_minimum_required(VERSION 3.2)
project(thermostat)

set(ENV{PATH} "/usr/local/sbin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin")
set(PLATFORMIO_CMD "/usr/local/bin/platformio")

include_directories("$ENV{HOME}/.platformio/packages/framework-arduinoespressif/variants/generic")
include_directories("$ENV{HOME}/.platformio/packages/framework-arduinoespressif/cores/esp8266")
include_directories("$ENV{HOME}/.platformio/packages/framework-arduinoespressif/cores/esp8266/spiffs")
include_directories("$ENV{HOME}/.platformio/packages/toolchain-xtensa/xtensa-lx106-elf/include")
include_directories("$ENV{HOME}/.platformio/packages/toolchain-xtensa/lib/gcc/xtensa-lx106-elf/4.8.2/include")
include_directories("$ENV{HOME}/.platformio/packages/toolchain-xtensa/lib/gcc/xtensa-lx106-elf/4.8.2/include-fixed")

add_definitions(-DF_CPU=40000000L)
add_definitions(-D__ets__)
add_definitions(-DICACHE_FLASH)
add_definitions(-DARDUINO_ESP8266_ESP01)
add_definitions(-DARDUINO_ARCH_ESP8266)
add_definitions(-DESP8266)
add_definitions(-DARDUINO=10605)
add_definitions(-DPLATFORMIO=020304)

add_custom_target(
    PLATFORMIO_BUILD ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    PLATFORMIO_UPLOAD ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run --target upload
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    PLATFORMIO_CLEAN ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run --target clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

#
# To enable code auto-completion, please specify path 
# to main source file (*.c, *.cpp) and uncomment line below
#
add_executable(thermostat src/main.cpp)
