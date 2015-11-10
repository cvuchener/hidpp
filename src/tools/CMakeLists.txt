cmake_minimum_required(VERSION 2.8)
project(hidpp_tools)

include_directories(../libhidpp)

add_executable(hidpp-test hidpp-test.cpp)
target_link_libraries(hidpp-test hidpp)

add_executable(hidpp-list-features hidpp-list-features.cpp)
target_link_libraries(hidpp-list-features hidpp)

add_executable(hidpp10-dump-page hidpp10-dump-page.cpp)
target_link_libraries(hidpp10-dump-page hidpp)

add_executable(hidpp10-write-page hidpp10-write-page.cpp)
target_link_libraries(hidpp10-write-page hidpp)

add_executable(hidpp10-raw-command hidpp10-raw-command.cpp)
target_link_libraries(hidpp10-raw-command hidpp)
