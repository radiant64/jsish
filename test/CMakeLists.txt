cmake_minimum_required(VERSION 3.0...3.31)
project(JsishTest)

add_executable(test test.c)
target_include_directories(test PRIVATE ..)
set_property(TARGET test PROPERTY C_STANDARD 90)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
	target_compile_options(test PRIVATE -Wall -Werror -pedantic)
endif()

