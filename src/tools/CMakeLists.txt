cmake_minimum_required(VERSION 2.8)
project(hidpp_tools)

include_directories(../libhidpp)

add_library(common OBJECT
	common/common.cpp
	common/Option.cpp
	common/CommonOptions.cpp)

add_executable(hidpp-check-device
	hidpp-check-device.cpp
	$<TARGET_OBJECTS:common>)
target_link_libraries(hidpp-check-device hidpp)

if(LIBUDEV_FOUND)
	add_executable(hidpp-list-devices
		hidpp-list-devices.cpp
		$<TARGET_OBJECTS:common>)
	target_include_directories(hidpp-list-devices
		PUBLIC ${LIBUDEV_INCLUDE_DIRECTORIES})
	target_link_libraries(hidpp-list-devices hidpp ${LIBUDEV_LIBRARIES})
else()
	message("hidpp-list-devices require libudev.")
endif()

add_executable(hidpp-list-features
	hidpp-list-features.cpp
	$<TARGET_OBJECTS:common>)
target_link_libraries(hidpp-list-features hidpp)

add_executable(hidpp10-dump-page
	hidpp10-dump-page.cpp
	$<TARGET_OBJECTS:common>)
target_link_libraries(hidpp10-dump-page hidpp)

add_executable(hidpp10-write-page
	hidpp10-write-page.cpp
	$<TARGET_OBJECTS:common>)
target_link_libraries(hidpp10-write-page hidpp)

add_executable(hidpp10-raw-command
	hidpp10-raw-command.cpp
	$<TARGET_OBJECTS:common>)
target_link_libraries(hidpp10-raw-command hidpp)

add_executable(hidpp10-mouse-resolution
	hidpp10-mouse-resolution.cpp
	$<TARGET_OBJECTS:common>)
target_link_libraries(hidpp10-mouse-resolution hidpp)

add_executable(hidpp10-active-profile
	hidpp10-active-profile.cpp
	$<TARGET_OBJECTS:common>)
target_link_libraries(hidpp10-active-profile hidpp)

if(TINYXML2_FOUND)
	add_library(profile OBJECT
		profile/ProfileXML.cpp
		profile/MacroText.cpp)
	
	add_executable(hidpp10-persistent-profiles
		hidpp10-persistent-profiles.cpp
		$<TARGET_OBJECTS:profile>
		$<TARGET_OBJECTS:common>)
	target_include_directories(hidpp10-persistent-profiles
		PUBLIC ${TINYXML2_INCLUDE_DIRECTORIES})
	target_link_libraries(hidpp10-persistent-profiles hidpp ${TINYXML2_LIBRARIES})
	
	add_executable(hidpp10-load-temp-profile
		hidpp10-load-temp-profile.cpp
		$<TARGET_OBJECTS:profile>
		$<TARGET_OBJECTS:common>)
	target_include_directories(hidpp10-load-temp-profile
		PUBLIC ${TINYXML2_INCLUDE_DIRECTORIES})
	target_link_libraries(hidpp10-load-temp-profile hidpp ${TINYXML2_LIBRARIES})
else()
	message("Profile tools require tinyxml2.")
endif()

add_executable(hidpp20-call-function
	hidpp20-call-function.cpp
	$<TARGET_OBJECTS:common>)
target_link_libraries(hidpp20-call-function hidpp)

add_executable(hidpp20-onboard-profiles-get-description
	hidpp20-onboard-profiles-get-description.cpp
	$<TARGET_OBJECTS:common>)
target_link_libraries(hidpp20-onboard-profiles-get-description hidpp)

add_executable(hidpp20-dump-page
	hidpp20-dump-page.cpp
	$<TARGET_OBJECTS:common>)
target_link_libraries(hidpp20-dump-page hidpp)
