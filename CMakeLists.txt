cmake_minimum_required (VERSION 3.8)

project(jsonman)

#Comment this definition when done with running the tests
add_definitions(-DJSONIC_TEST=1)

add_subdirectory (src)

enable_testing ()
add_subdirectory (test)