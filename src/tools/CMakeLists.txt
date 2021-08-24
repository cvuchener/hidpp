cmake_minimum_required(VERSION 3.18)
project(hidpp_tools)

if (WIN32 AND NOT MINGW)
	find_path(GETOPT_INCLUDE_DIR getopt.h REQUIRED)
	find_library(GETOPT_LIBRARY getopt REQUIRED)
	add_library(getopt SHARED IMPORTED)
	set_target_properties(getopt PROPERTIES
		IMPORTED_IMPLIB ${GETOPT_LIBRARY}
		INTERFACE_INCLUDE_DIRECTORIES ${GETOPT_INCLUDE_DIR})
endif()

add_library(common OBJECT
	common/common.cpp
	common/Option.cpp
	common/CommonOptions.cpp)
target_link_libraries(common PUBLIC hidpp $<$<TARGET_EXISTS:getopt>:getopt>)

add_executable(hidpp-check-device hidpp-check-device.cpp)
target_link_libraries(hidpp-check-device hidpp common Threads::Threads)
install(TARGETS hidpp-check-device RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

set(TOOLS
	hidpp-list-devices
	hidpp-list-features
	hidpp-mouse-resolution
	hidpp10-dump-page
	hidpp10-write-page
	hidpp10-raw-command
	hidpp10-active-profile
	hidpp20-call-function
	hidpp20-onboard-profiles-get-description
	hidpp20-reprog-controls
	hidpp20-led-control
	hidpp20-dump-page
	hidpp20-write-page
	hidpp20-write-data
)
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(TOOLS ${TOOLS}
		hidpp20-mouse-event-test
		hidpp20-raw-touchpad-driver
	)
endif()

foreach(TOOL_NAME ${TOOLS})
	add_executable(${TOOL_NAME} ${TOOL_NAME}.cpp)
	target_link_libraries(${TOOL_NAME}
		hidpp
		common
		Threads::Threads
	)
	install(TARGETS ${TOOL_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()

find_package(tinyxml2)
if(tinyxml2_FOUND)
	add_library(profile OBJECT
		profile/MacroText.cpp
		profile/ProfileXML.cpp)
	target_link_libraries(profile PUBLIC hidpp tinyxml2::tinyxml2)
	
	foreach(TOOL_NAME
		hidpp-persistent-profiles
		hidpp10-load-temp-profile
	)
		add_executable(${TOOL_NAME} ${TOOL_NAME}.cpp)
		target_link_libraries(${TOOL_NAME}
			hidpp
			profile
			common
			tinyxml2::tinyxml2
			Threads::Threads
		)
		install(TARGETS ${TOOL_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
	endforeach()
	
else()
	message("Profile tools require tinyxml2.")
endif()
