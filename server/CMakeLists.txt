cmake_minimum_required(VERSION 3.25.1)

project(ChatroomServer)

add_executable(main.out main.c)

set_target_properties(${TARGET} PROPERTIES
COMPILE_FLAGS "-pthread"
LINK_FLAGS "-pthread")