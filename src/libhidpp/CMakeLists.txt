cmake_minimum_required(VERSION 2.8)
project(libhidpp)

set(LIBHIDPP_SOURCES
	misc/HIDRaw.cpp
	misc/SysCallError.cpp
	hidpp/Device.cpp
	hidpp/Parameters.cpp
	hidpp/Report.cpp
	hidpp10/Device.cpp
	hidpp10/Error.cpp
	hidpp10/WriteError.cpp
	hidpp10/IMemory.cpp
	hidpp20/Device.cpp
	hidpp20/Error.cpp
	hidpp20/IRoot.cpp
	hidpp20/IFeatureSet.cpp
)

include_directories(.)
add_library(hidpp SHARED ${LIBHIDPP_SOURCES})
