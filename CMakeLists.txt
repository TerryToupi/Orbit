cmake_minimum_required(VERSION 3.25)

project(Orbit
	VERSION 0.0.1
	DESCRIPTION "A game engine"
	LANGUAGES C CXX
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(cmake/options.cmake)

set(CMAKE_FOLDER "vendor")
add_subdirectory(vendor)
unset(CMAKE_FOLDER)

add_subdirectory(runtime)
