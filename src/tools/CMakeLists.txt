cmake_minimum_required(VERSION 2.8)
project(hidpp_tools)

include_directories(../libhidpp)

set(COMMON_SOURCES
	common/common.cpp
	common/Option.cpp
	common/CommonOptions.cpp
)

add_library(common OBJECT ${COMMON_SOURCES})

add_executable(hidpp-test hidpp-test.cpp $<TARGET_OBJECTS:common>)
target_link_libraries(hidpp-test hidpp)

if(LIBUDEV_FOUND)
	add_executable(hidpp-list-devices hidpp-list-devices.cpp $<TARGET_OBJECTS:common>)
	target_include_directories(hidpp-list-devices PUBLIC ${LIBUDEV_INCLUDE_DIRECTORIES})
	target_link_libraries(hidpp-list-devices hidpp ${LIBUDEV_LIBRARIES})
endif()

add_executable(hidpp-list-features hidpp-list-features.cpp $<TARGET_OBJECTS:common>)
target_link_libraries(hidpp-list-features hidpp)

add_executable(hidpp10-dump-page hidpp10-dump-page.cpp $<TARGET_OBJECTS:common>)
target_link_libraries(hidpp10-dump-page hidpp)

add_executable(hidpp10-write-page hidpp10-write-page.cpp $<TARGET_OBJECTS:common>)
target_link_libraries(hidpp10-write-page hidpp)

add_executable(hidpp10-raw-command hidpp10-raw-command.cpp $<TARGET_OBJECTS:common>)
target_link_libraries(hidpp10-raw-command hidpp)

add_executable(hidpp10-mouse-resolution hidpp10-mouse-resolution.cpp $<TARGET_OBJECTS:common>)
target_link_libraries(hidpp10-mouse-resolution hidpp)

add_executable(hidpp10-active-profile hidpp10-active-profile.cpp $<TARGET_OBJECTS:common>)
target_link_libraries(hidpp10-active-profile hidpp)

if(TINYXML2_FOUND)
	add_library(profile OBJECT
		profile/ProfileXML.cpp
		profile/MacroText.cpp)
	
	add_executable(hidpp10-read-profiles hidpp10-read-profiles.cpp $<TARGET_OBJECTS:profile> $<TARGET_OBJECTS:common>)
	target_include_directories(hidpp10-read-profiles PUBLIC ${TINYXML2_INCLUDE_DIRECTORIES})
	target_link_libraries(hidpp10-read-profiles hidpp ${TINYXML2_LIBRARIES})
	
	add_executable(hidpp10-write-profiles hidpp10-write-profiles.cpp $<TARGET_OBJECTS:profile> $<TARGET_OBJECTS:common>)
	target_include_directories(hidpp10-write-profiles PUBLIC ${TINYXML2_INCLUDE_DIRECTORIES})
	target_link_libraries(hidpp10-write-profiles hidpp ${TINYXML2_LIBRARIES})

	add_executable(hidpp10-load-temp-profile hidpp10-load-temp-profile.cpp $<TARGET_OBJECTS:profile> $<TARGET_OBJECTS:common>)
	target_include_directories(hidpp10-load-temp-profile PUBLIC ${TINYXML2_INCLUDE_DIRECTORIES})
	target_link_libraries(hidpp10-load-temp-profile hidpp ${TINYXML2_LIBRARIES})
else()
	message("hidpp10-read-profiles require tinyxml2.")
endif()
