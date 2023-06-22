cmake_minimum_required(VERSION 3.25.1)

project(ChatroomClient)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(main.out main.c)
target_link_libraries(main.out ${CURSES_LIBRARIES})

set_target_properties(${TARGET} PROPERTIES
COMPILE_FLAGS "-pthread"
LINK_FLAGS "-pthread")