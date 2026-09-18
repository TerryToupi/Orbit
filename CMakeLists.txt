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

option(ORBIT_BUILD_TESTS "Build runtime foundation tests" OFF)
option(ORBIT_BUILD_GPU_TESTS "Build compositor tests requiring a GPU and window system" OFF)
option(ORBIT_BUILD_EXAMPLES "Build standalone runtime examples" ON)
if(ORBIT_BUILD_TESTS)
	enable_testing()
endif()

set(CMAKE_FOLDER "vendor")
add_subdirectory(vendor)
unset(CMAKE_FOLDER)

add_subdirectory(runtime)

if(ORBIT_BUILD_EXAMPLES)
	add_subdirectory(examples/rendering_demo)
endif()

if(ORBIT_BUILD_TESTS)
	add_subdirectory(tests)
endif()
