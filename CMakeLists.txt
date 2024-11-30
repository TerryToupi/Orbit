#https://github.com/meemknight/cmakeSetup
#Version 1.1.0

cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)

#! ! ! ! ! ! !
#set this to true to ship the game!
#basically this will change the RESOURCES_PATH to be the local path
#! ! ! ! ! ! !
#DELETE THE OUT FOLDER AFTER CHANGING THIS BECAUSE VISUAL STUDIO DOESN'T SEEM TO RECOGNIZE THIS CHANGE AND REBUILD!
option(PRODUCTION "Make this a production build" OFF)
#DELETE THE OUT FOLDER AFTER CHANGING THIS BECAUSE VISUAL STUDIO DOESN'T SEEM TO RECOGNIZE THIS CHANGE AND REBUILD!

# Automatically set Production ON for Release builds
if (CMAKE_BUILD_TYPE STREQUAL "Release")
	set(PRODUCTION ON CACHE BOOL "Enable production mode" FORCE)
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(PRODUCTION OFF CACHE BOOL "Enable production mode" FORCE)
endif()

# Select one of the following backends. ONLY ONE MUST BE SELECTED
option(VULKAN_BACKEND "Select Vulkan as the native backend" ON)
option(WEBGPU_BACKEND "Select Vulkan as the native backend" OFF)
option(D3D12_BACKEND "Select Vulkan as the native backend" 0FF)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>") 
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) #link time optimization

if(MSVC)
	#make sure SIMD optimizations take place
	add_compile_options(/arch:AVX2)
endif()

project(Orbit)

# import external libs
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(External/glfw-3.4)
add_subdirectory(External/spdlog-1.15.0)
add_subdirectory(External/volk-1.3.295)
add_subdirectory(External/Vulkan-Headers)  
add_subdirectory(External/VulkanMemoryAllocator-3.1.0) 
add_subdirectory(External/stb_image)

function(assign_source_groups base_dir)
    # Collect only .cpp and .h files recursively
    file(GLOB_RECURSE ALL_FILES
        "${base_dir}/*.cpp"
        "${base_dir}/*.h"
    )

    foreach(file IN LISTS ALL_FILES)
        # Get the relative path from the base directory
        file(RELATIVE_PATH rel_path "${base_dir}" "${file}")

        # Extract the directory part of the relative path
        get_filename_component(group_path "${rel_path}" DIRECTORY)

        # Replace '/' with '\\' for Windows compatibility
        string(REPLACE "/" "\\" group_path "${group_path}")

        # Assign the file to the appropriate source group
        source_group("${group_path}" FILES "${file}")
    endforeach()
endfunction() 

# making filters to be used for easier navigation inside Xcode or VS solution (if you open it that way)
assign_source_groups("${CMAKE_SOURCE_DIR}")

# MY_SOURCES is defined to be a list of all the source files for my game 
# DON'T ADD THE SOURCES BY HAND, they are already added with this macro
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Engine/**.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Engine/**.h")
file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Editor/**.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Editor/**.h")
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/App/**.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/App/**.h")

# executable stage
add_executable("${CMAKE_PROJECT_NAME}")

set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 20)


# -------------------------------- TARGET DEFINITIONS ---------------------------------------
target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC GLFW_INCLUDE_NONE) 

if(PRODUCTION AND MSVC)
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)

	# setup the ASSETS_PATH macro to be in the root folder of your exe
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC ASSETS_PATH="./Assets/") 

	# no console
	set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
	
	# remove the option to debug asserts.
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION) 

	# Define Variable for windows	
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC OP_WINDOWS) 

elseif(PRODUCTION AND APPLE)
	# setup the ASSETS_PATH macro to be in the root folder of your exe
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC ASSETS_PATH="./Assets/") 

	# remove the option to debug asserts.
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION)

	# Define Variable for macos 
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC OP_MACOS)

elseif(MSVC) 
	# This is useful to get an ASSETS_PATH in your IDE during development
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/App/Assets/")

	# asserting the DEBUG variable
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEBUG) 

	# Define Variable for windows	
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC OP_WINDOWS)

elseif(APPLE) 
	# This is useful to get an ASSETS_PATH in your IDE during development
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/App/Assets/")

	# asserting the DEBUG variable
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEBUG) 

	# Define Variable for windows	
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC OP_MACOS)

endif()

if (VULKAN_BACKEND)
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC VULKAN_BACKEND)  

elseif(WEBGPU_BACKEND)
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC WEBGPU_BACKEND)   

elseif(D3D12_BACKEND)
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC D3D12_BACKEND)

endif() 
# -------------------------------- TARGET DEFINITIONS --------------------------------------- 


target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${ENGINE_SOURCES} ${EDITOR_SOURCES} ${APP_SOURCES})

target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Engine/")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Editor/")
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/App/") 

target_link_libraries(
	"${CMAKE_PROJECT_NAME}" 
	PRIVATE 
		glfw 
		spdlog 
		volk 
		Vulkan-Headers
		VulkanMemoryAllocator
		stb_image
)