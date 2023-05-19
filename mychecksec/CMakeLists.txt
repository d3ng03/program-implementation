cmake_minimum_required(VERSION 3.10)

project(mychecksec)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
add_executable(mychecksec src/mychecksec.c)

install(TARGETS mychecksec DESTINATION /usr/local/bin)
