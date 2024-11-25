cmake_minimum_required(VERSION 3.16)
project(stb_image LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the library
add_library(stb_image STATIC)

target_include_directories(stb_image PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Source files
target_sources(stb_image PRIVATE
    stb_image.cpp
)