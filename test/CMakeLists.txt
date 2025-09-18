cmake_minimum_required(VERSION 3.0...3.31)
project(JsishTests)

add_executable(test-decode test-decode.c)
target_include_directories(test-decode PRIVATE ..)
set_property(TARGET test-decode PROPERTY C_STANDARD 90)

if (CMAKE_C_COMPILER_ID STREQUAL "gnu" OR CMAKE_C_COMPILER_ID STREQUAL "clang")
	target_compile_options(test-decode PRIVATE -Wall -Werror -pedantic)
endif()

