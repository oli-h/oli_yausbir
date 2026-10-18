cmake_minimum_required(VERSION 3.10.0)
project(HelloOli VERSION 0.1.0 LANGUAGES C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED IMPORTED_TARGET libusb-1.0)
pkg_check_modules(LIBLIRC REQUIRED IMPORTED_TARGET lirc)

# file(GLOB SOURCES "*.c")
# add_library(oli_yausbir SHARED ${SOURCES})
add_library(oli_yausbir SHARED oli_yausbir.c)

target_include_directories(oli_yausbir PRIVATE /usr/include/lirc)

target_link_libraries(oli_yausbir PRIVATE PkgConfig::LIBUSB)
