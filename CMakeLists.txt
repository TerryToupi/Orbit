#https://github.com/meemknight/cmakeSetup
#Version 1.1.0

cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)

#! ! ! ! ! ! !
#set this to true to ship the game!
#basically this will change the RESOURCES_PATH to be the local path
#! ! ! ! ! ! !
#DELETE THE OUT FOLDER AFTER CHANGING THIS BECAUSE VISUAL STUDIO DOESN'T SEEM TO RECOGNIZE THIS CHANGE AND REBUILD!
option(PRODUCTION_BUILD "Make this a production build" OFF)
#DELETE THE OUT FOLDER AFTER CHANGING THIS BECAUSE VISUAL STUDIO DOESN'T SEEM TO RECOGNIZE THIS CHANGE AND REBUILD! 

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>") 
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) #link time optimization

if(MSVC) 
add_compile_options(/arch:AVX2) #make sure SIMD optimizations take place
endif()

project(Orbit)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(External/glfw-3.4)
add_subdirectory(External/spdlog-1.15.0)
add_subdirectory(External/volk-1.3.295)
add_subdirectory(External/Vulkan-Headers)

# MY_SOURCES is defined to be a list of all the source files for my game 
# DON'T ADD THE SOURCES BY HAND, they are already added with this macro
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Engine/src/**.cpp")
file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Editor/src/**.cpp")
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/App/src/**.cpp")

add_executable("${CMAKE_PROJECT_NAME}")

set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 20)

target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC GLFW_INCLUDE_NONE=1) 

if(PRODUCTION_BUILD)
	# setup the ASSETS_PATH macro to be in the root folder of your exe
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/") 

	# remove the option to debug asserts.
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1) 

else()
	# This is useful to get an ASSETS_PATH in your IDE during development
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0)  

endif()

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${ENGINE_SOURCES} ${EDITOR_SOURCES} ${APP_SOURCES})

if(MSVC) # If using the VS compiler...

	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)

	set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") #no console  

endif()

target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Engine/src/")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Editor/src/")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/App/src/")

target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEBUG) 
target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC OP_WINDOWS) 
target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC VULKAN_BACKEND) 

target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE glfw 
    spdlog volk Vulkan-Headers)