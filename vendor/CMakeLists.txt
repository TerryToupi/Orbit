cmake_minimum_required(VERSION 3.25)
project(vendor)

include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")

FetchContent_Declare(
	SDL
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SDL
	OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(SDL)

FetchContent_Declare(
	box3d	
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/box3d
	OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(box3d)
