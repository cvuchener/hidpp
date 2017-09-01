cmake_minimum_required(VERSION 3.1)
project(hidpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(DEFAULT_HID_BACKEND "linux")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(DEFAULT_HID_BACKEND "windows")
else()
	message(WARNING "System is not supported")
endif()
set(HID_BACKEND "${DEFAULT_HID_BACKEND}" CACHE STRING "Backend used for accessing HID devices")
set_property(CACHE HID_BACKEND PROPERTY STRINGS linux windows)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(TINYXML2 tinyxml2)
if("${HID_BACKEND}" STREQUAL "linux")
	pkg_check_modules(LIBUDEV libudev REQUIRED)
elseif("${HID_BACKEND}" STREQUAL "windows")
	add_definitions(-DUNICODE -D_UNICODE)
	add_definitions(-D_WIN32_WINNT=0x0600) # Use vista or later
else()
	message(FATAL_ERROR "HID_BACKEND is invalid.")
endif()

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
