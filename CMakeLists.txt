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

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if (MSVC OR APPLE OR UNIX)
	option(ORBIT_SANITIZE "Enable sanitizers for some builds" OFF)
	set(ORBIT_SANITIZER_TYPE "address" CACHE STRING "Type of sanitizer to use (address, thread, undefined, memory)")
	set_property(CACHE ORBIT_SANITIZER_TYPE PROPERTY STRINGS "address" "thread" "undefined" "memory")

	if(ORBIT_SANITIZE)
	message(STATUS "Orbit Sanitize: ${ORBIT_SANITIZER_TYPE}")
		# sanitizers need to apply to all compiled libraries to work well
		if(MSVC)
			# address sanitizer only in the debug build
			add_compile_options("$<$<CONFIG:Debug>:/fsanitize=address>")
			add_link_options("$<$<CONFIG:Debug>:/INCREMENTAL:NO>")
		elseif(APPLE)
			# more sanitizers on Apple clang
			# UBSan recovers by default, no-recover makes the first report fatal so CI fails
			if(ORBIT_SANITIZER_TYPE STREQUAL "thread")
				add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
				add_link_options(-fsanitize=thread)
			elseif(ORBIT_SANITIZER_TYPE STREQUAL "undefined")
				add_compile_options(-fsanitize=undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
				add_link_options(-fsanitize=undefined)
			else()
				# default to address sanitizer
				add_compile_options(-fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined -fno-sanitize-recover=all)
				add_link_options(-fsanitize=address -fsanitize-address-use-after-scope -fsanitize=undefined)
			endif()
		elseif(UNIX)
			# Linux/WSL2 sanitizer support
			if(ORBIT_SANITIZER_TYPE STREQUAL "thread")
				# Disable ASLR first to avoid crash on launch
				# echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
				add_compile_options(-fsanitize=thread -fno-omit-frame-pointer -fPIE)
				add_link_options(-fsanitize=thread -pie)
			elseif(ORBIT_SANITIZER_TYPE STREQUAL "undefined")
				add_compile_options(-fsanitize=undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
				add_link_options(-fsanitize=undefined)
			elseif(ORBIT_SANITIZER_TYPE STREQUAL "memory")
				add_compile_options(-fsanitize=memory -fno-omit-frame-pointer)
				add_link_options(-fsanitize=memory)
			else()
				# default to address sanitizer
				add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
				add_link_options(-fsanitize=address)
			endif()
		endif()
	else()
		if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND PROJECT_IS_TOP_LEVEL)
			# enable hot reloading
			add_compile_options("$<$<CONFIG:Debug>:/ZI>")
			add_link_options("$<$<CONFIG:Debug>:/INCREMENTAL>")
		endif()
	endif()
endif()

block()
set(CMAKE_FOLDER "vendor")
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendor/box3d EXCLUDE_FROM_ALL)
endblock()

set(ORBIT_SRC_FILES
	src/core.hpp
	src/platform.hpp
    src/platform.cpp
    src/program.cpp
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${ORBIT_SRC_FILES})

add_executable(${PROJECT_NAME} ${ORBIT_SRC_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3)
target_link_libraries(${PROJECT_NAME} PUBLIC box3d::box3d)
