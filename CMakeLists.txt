cmake_minimum_required(VERSION 3.1)
project(hidpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(TINYXML2 tinyxml2)
pkg_check_modules(LIBUDEV libudev)

add_subdirectory(src/libhidpp)
add_subdirectory(src/tools)
add_subdirectory(doc/libhidpp)

set(PREFIX ${CMAKE_INSTALL_PREFIX})

configure_file(pkgconfig/hidpp.pc.in pkgconfig/hidpp.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/pkgconfig/hidpp.pc DESTINATION share/pkgconfig)

option(INSTALL_UDEV_RULES "Install udev rules for user access to HID++ devices" OFF)
if(INSTALL_UDEV_RULES)
	configure_file(udev/rules.d/69-hidpp.rules.in udev/rules.d/69-hidpp.rules @ONLY)
	install(FILES ${CMAKE_BINARY_DIR}/udev/rules.d/69-hidpp.rules DESTINATION /etc/udev/rules.d)
endif()
