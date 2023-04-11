cmake_minimum_required(VERSION 3.10)

project(g-cat)

add_executable(g-cat src/g-cat.c)

install(TARGETS g-cat DESTINATION /usr/local/bin)