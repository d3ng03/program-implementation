cmake_minimum_required(VERSION 3.10)

project(parse)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
add_executable(parser src/parser-impl.c)

install(TARGETS parser DESTINATION /usr/local/bin)