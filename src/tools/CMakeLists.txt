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

add_executable(hidpp-list-features hidpp-list-features.cpp $<TARGET_OBJECTS:common>)
target_link_libraries(hidpp-list-features hidpp)

add_executable(hidpp10-dump-page hidpp10-dump-page.cpp $<TARGET_OBJECTS:common>)
target_link_libraries(hidpp10-dump-page hidpp)

add_executable(hidpp10-write-page hidpp10-write-page.cpp $<TARGET_OBJECTS:common>)
target_link_libraries(hidpp10-write-page hidpp)

add_executable(hidpp10-raw-command hidpp10-raw-command.cpp $<TARGET_OBJECTS:common>)
target_link_libraries(hidpp10-raw-command hidpp)
